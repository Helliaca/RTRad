import Scene.Raytracing;
import Scene.Shading;
import Utils.Sampling.TinyUniformSampleGenerator;
import Experimental.Scene.Lights.LightHelpers;
import Experimental.Scene.Material.StandardMaterial;

import RTRad.RTRad.Slang.VisCaching;
import RTRad.RTRad.Slang.Voxel;
import RTRad.RTRad.Slang.Random;
import RTRad.RTRad.Slang.HemisphericSampling;
import RTRad.RTRad.Slang.Hemispheric;

#define PI 3.14159265359f
#define max_bufferpos 4294705152

#define ref 0.9f

// Texture-Group
Texture2D<float4> pos;      // position
Texture2D<float4> nrm;      // normal
Texture2D<float> arf;       // surface area
Texture2D<float4> mat;      // material/color
Texture2D<float4> lig;      // lighting-input
RWTexture2D<float4> lig2;   // lighting-output
Texture3D<float4> voxTex;   // voxel-map

// vis-caching buffer
RWBuffer<uint> vis : register(t9);

// Uniforms
cbuffer PerFrameCB {
    uint2 currentOffset;

    int sampling_res;
    int passNum;
    int texRes;

    bool randomizeSamples;
    bool useSubstructuring;

    float3 minPos;
    float3 maxPos;

    int voxelRaymarchRatio;
};

SamplerState sampleWrap : register(s0);

struct RayPayload
{
    uint2 self_c;
    uint2 other_c;
};

[shader("raygeneration")]
void rayGen()
{
    uint2 self_c = DispatchRaysIndex().xy + currentOffset;

    // If pos alpha is less than 1, skip this.
    //if (pos[self_c].a < 1.0f) {
    //    return;
    //}

    // World position of current texel
    float3 self_wpos = pos[self_c].xyz + minPos;

    float dim1;
    float dim2;
    pos.GetDimensions(dim1, dim2);

    uint matID = (uint) mat[self_c].a;
    float3 emissive = gScene.materials[matID].emissive;
    //if (length(emissive) > 0.1f) return; // don't run for light-sources

    // Use this to display a cool way of what points rayMarch determines to be visible from a given point
    /*lig2[self_c] = float4(0, 0, 0, 1);
    if (rayMarchVisible(self_wpos, float3(0.f, 0.f, -1.0f))) {
        lig2[self_c] = float4(1,1,1,1);
    }
    return;*/

    #if HEMISPHERIC

    float3 surface_normal = 2.0f * (nrm[self_c].xyz - 0.5f);
    surface_normal = normalize(surface_normal);

    float3 pe = float3(0, 0, 1);

    if (abs(surface_normal.z) > 0.99f) {
        pe = float3(0.01f, 0, 0.99f);
    }

    float3 bitangent = simplePerp(surface_normal, pe);
    float3 tangent = simplePerp(bitangent, surface_normal);

    float3x3 m = {
        bitangent,
        tangent,
        surface_normal,
    };
    //m = transpose(m);

    for (int i = 0; i < 100; i++) {
        float3 rv = mul(m, sampledirs[i]);
        //float3 rv = getCosHemisphereSample(seed, surface_normal);
        //float3 rv = mul(m, sampledirs[i]);
        //make_ray(self_wpos, rv, self_c);
        //lig2[self_c].rgb = rv;

        //float3 surface_normal = 2.0f * (nrm[self_c].xyz - 0.5f);

        RayDesc ray;
        ray.Origin = self_wpos;

        //ray.Direction = normalize(surface_normal);
        //dirv = toForward(dirv, surface_normal);
        ray.Direction = rv;

        ray.TMin = 0.01f;
        ray.TMax = 10000;

        RayPayload rpl = { self_c, uint2(0,0) };

        TraceRay(gScene.rtAccel,                        // A Falcor built-in containing the raytracing acceleration structure
            RAY_FLAG_CULL_BACK_FACING_TRIANGLES,  // Ray flags.  (Here, we will skip hits with back-facing triangles)
            0xFF,                                 // Instance inclusion mask.  0xFF => no instances discarded from this mask
            0,                                    // Hit group to index (i.e., when intersecting, call hit shader #0)
            0,//hitProgramCount,                      // Number of hit groups ('hitProgramCount' is built-in from Falcor with the right number)
            0,                                    // Miss program index (i.e., when missing, call miss shader #0)
            ray,                                  // Data structure describing the ray to trace
            rpl
        );
    }
    return;

    #endif

    for (uint x = 0; x < dim1; x += sampling_res) {
        for (uint y = 0; y < dim2; y += sampling_res) {
            uint2 other_c = uint2(x, y);

            if (pos[other_c].a < 1.0f || (useSubstructuring && lig[other_c].a < 1.0f)) continue;


            // Viscaching and randomization are mutually exclusive.
            #if VISCACHE
            if (passNum > 0) {
                // Get viscache
                uint bufPos = getBufferPos(self_c, other_c, texRes);

                if (bufPos <= max_bufferpos) {
                    if (getVisible(bufPos, vis)) {
                        setColor(self_c, other_c);
                    }
                    continue;
                }
                lig2[self_c] = float4(1, 1, 1, 1);
                continue;
            }
            #elif RANDOMIZE
            if (randomizeSamples) {
                uint2 seed = uint2(
                    random((other_c.x + 1) * (other_c.y + 1) + passNum, 7864128),
                    random((other_c.x + 1) * (other_c.y + 1) + passNum, 5490141)
                    );

                uint2 rnd = random(seed, uint2(sampling_res, sampling_res));

                other_c += uint2(
                    rnd.x,
                    rnd.y
                    );
            }
            #endif

            if (other_c.x - self_c.x < sampling_res && other_c.y - self_c.y < sampling_res) continue;

            float3 other_wpos = pos[other_c].xyz + minPos;

            #if VOXELRAYMARCH
            if ((other_c.x + other_c.y * dim2) % voxelRaymarchRatio == 0) {
                if (vRayMarch(self_wpos, other_wpos, voxTex, minPos, maxPos)) {
                    setColor(self_c, other_c);
                }
                continue;
            }
            #endif

            RayDesc ray;
            ray.Origin = self_wpos;
            ray.Direction = normalize(other_wpos - self_wpos);
            ray.TMin = 0.0001f;
            ray.TMax = distance(self_wpos, other_wpos) - (2.0f * ray.TMin);

            RayPayload rpl = { self_c, other_c };

            TraceRay(gScene.rtAccel,                        // A Falcor built-in containing the raytracing acceleration structure
                RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH | RAY_FLAG_SKIP_CLOSEST_HIT_SHADER,  // Ray flags.  (Here, we will skip hits with back-facing triangles)
                0xFF,                                 // Instance inclusion mask.  0xFF => no instances discarded from this mask
                0,                                    // Hit group to index (i.e., when intersecting, call hit shader #0)
                0,//hitProgramCount,                      // Number of hit groups ('hitProgramCount' is built-in from Falcor with the right number)
                0,                                    // Miss program index (i.e., when missing, call miss shader #0)
                ray,                                  // Data structure describing the ray to trace
                rpl
            );
        }
    }
}

[shader("miss")]
void primaryMiss(inout RayPayload rpl)
{
    #if HEMISPHERIC
    return;
    #endif

    uint2 self_c = rpl.self_c;
    uint2 other_c = rpl.other_c;

    #if VISCACHE
    uint bufPos = getBufferPos(self_c, other_c, texRes);
    if (bufPos <= max_bufferpos) {
        setVisible(bufPos, vis);
    }
    #endif

    setColor(self_c, other_c);
}

void setColor(uint2 self_c, uint2 other_c) {
    float3 self_wpos = pos[self_c].xyz + minPos;
    float3 other_wpos = pos[other_c].xyz + minPos;

    float3 self_to_other = other_wpos - self_wpos;

    float r = length(self_to_other);

    // Form factor
    self_to_other = normalize(self_to_other);

    float3 self_nrm = 2.0f * (nrm[self_c].xyz - 0.5f);
    float3 other_nrm = 2.0f * (nrm[other_c].xyz - 0.5f);

    float self_cos = dot(self_nrm, self_to_other);
    float other_cos = dot(other_nrm, -self_to_other);

    if (self_cos <= 0.0f || other_cos <= 0.0f) return;

    float F = self_cos * other_cos * (1.0f / (PI * r * r));
    F = min(1.0f, F); // Limiting view factor to 1.0f

    // Lighting
    float other_surface = arf[other_c].r; // surface area of other

    float4 self_color = float4(mat[self_c].rgb, 1.0f);

    #if MIPMAPPED_UNDERSAMPLING
    float dim1;
    float dim2;
    pos.GetDimensions(dim1, dim2);
    float ha = float(sampling_res) * 0.5f;
    float2 uvs = float2(float(other_c.x)+ha, float(other_c.y)+ha) / float2(float(dim1), float(dim2));
    float4 other_lig = lig.SampleLevel(sampleWrap, uvs, log2(sampling_res));
    #else
    float4 other_lig = lig[other_c];
    #endif

    lig2[self_c] += (sampling_res * sampling_res) * (lig[other_c].a * other_lig * self_color * ref * F * other_surface);
    lig2[self_c].a = 1.0f;
}

[shader("closesthit")]
void primaryClosestHit(inout RayPayload rpl, in BuiltInTriangleIntersectionAttributes attribs)
{
    VertexData v = getVertexData(getGeometryInstanceID(), PrimitiveIndex(), attribs);

    uint2 self_c = rpl.self_c;

    float2 uv = v.texC;

    float3 self_wpos = pos[self_c].xyz + minPos;
    float3 other_wpos = pos.SampleLevel(sampleWrap, uv, 0).xyz + minPos;

    float3 self_to_other = other_wpos - self_wpos;

    float r = length(self_to_other);

    if (r < 0.1f) return;

    self_to_other = normalize(self_to_other);

    float3 self_nrm = 2.0f * (nrm[self_c].xyz - 0.5f);
    float3 other_nrm = 2.0f * (nrm.SampleLevel(sampleWrap, uv, 0).xyz - 0.5f);

    float self_cos = dot(self_nrm, self_to_other);
    float other_cos = dot(other_nrm, -self_to_other);

    if (self_cos <= 0.0f) return;

    float view_factor = self_cos * (1.0f / (PI * max(r * r, 0.1f)));

    float4 col = lig.SampleLevel(sampleWrap, uv, 1);

    float4 self_color = float4(mat[rpl.self_c].rgb, 1.0f);

    lig2[self_c] += (col * self_color * ref * view_factor) / 100;
}

[shader("anyhit")]
void primaryAnyHit(inout RayPayload rpl, BuiltInTriangleIntersectionAttributes attribs)
{

}
