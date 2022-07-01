import Scene.Raytracing;
import Scene.Shading;
import Utils.Sampling.TinyUniformSampleGenerator;
import Experimental.Scene.Lights.LightHelpers;
import Experimental.Scene.Material.StandardMaterial;

#if VISCACHE
import RTRad.RTRad.Slang.VisCaching;
#endif

#if VOXELRAYMARCH
import RTRad.RTRad.Slang.Voxel;
#endif

#if RANDOMIZE
import RTRad.RTRad.Slang.Random;
#endif

#if HEMISPHERIC
import RTRad.RTRad.Slang.HemisphericSampling;
import RTRad.RTRad.Slang.Hemispheric;
#endif


#define PI 3.14159265359f
#define max_bufferpos 4294705152


// Texture-Group
Texture2D<float4> pos;      // position
Texture2D<float4> nrm;      // normal
Texture2D<float> arf;       // surface area
Texture2D<float4> mat;      // material/color
Texture2D<float4> lig_in;      // lighting-input
RWTexture2D<float4> lig_out;   // lighting-output
Texture3D<float4> voxTex;   // voxel-map


// vis-caching buffer
RWBuffer<uint> vis : register(t9);


// Uniforms
cbuffer PerFrameCB {
    // What pixel in the output texture do we start at
    uint2 currentOffset;

    // Whats the sampling resolution (eg, sample one patch every RxR pixels
    int sampling_res;

    // Number of pass
    int passNum;

    // Resolution of the lightmap texture on one axis. Lightmaps are assumed to be squares
    int texRes;

    // Use an adaptive subdivision embeded within the alpha channel of the lig texture
    bool useSubstructuring;

    // Corners of the scenes bounding box
    float3 minPos;
    float3 maxPos;

    // Ration of rays to replace by voxel-raymarches. Every x'th ray will be replaced.
    int voxelRaymarchRatio;

    // Factor of reflectiviy rho
    float reflectivity_factor;

    // Factor of distance. Inverse square attenuation will be scaled by this factor
    float distance_factor;

    // Amount of directional samples to take in hemispherical mode
    uint hemisphere_samples;
};


// Texture sampler
SamplerState sampleWrap : register(s0);


// Ray payload. The target coordinate is not required for hemispheric sampling
struct RayPayload
{
    uint2 self_c;
#if HEMISPHERIC
#else
    uint2 other_c;
#endif
};


// RAY GENERATION
[shader("raygeneration")]
void rayGen()
{
    uint2 self_c = DispatchRaysIndex().xy + currentOffset;

    // If pos alpha is less than 1, skip this.
    if (pos[self_c].a < 1.0f) {
        return;
    }

    // World position of current texel
    float3 self_wpos = pos[self_c].xyz + minPos;

    uint matID = (uint) mat[self_c].a;
    float3 emissive = gScene.materials[matID].emissive;
    //if (length(emissive) > 0.1f) return; // don't run for light-sources

    // Use this to display a cool way of what points rayMarch determines to be visible from a given point
    /*lig_out[self_c] = float4(0, 0, 0, 1);
    if (rayMarchVisible(self_wpos, float3(0.f, 0.f, -1.0f))) {
        lig_out[self_c] = float4(1,1,1,1);
    }
    return;*/

    #if HEMISPHERIC

    //// HEMISPHERIC SAMPLING

    // Matrix to convert from tangent space to world space
    float3x3 m = getTangentToWorldMatrix(nrm[self_c].xyz);

    for (int i = 0; i < hemisphere_samples; i++) {
        RayDesc ray;
        ray.Origin = self_wpos;

        // Get ray direction. sampledirs is defined in HemisphericSampling.slang and includes 512 tangent space sampling directions.
        ray.Direction = mul(m, sampledirs[i]);

        ray.TMin = 0.01f;
        ray.TMax = 10000;

        RayPayload rpl = { self_c };

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

    #else

    //// CLASSIC SAMPLING

    for (uint x = 0; x < texRes; x += sampling_res) {
        for (uint y = 0; y < texRes; y += sampling_res) {
            uint2 other_c = uint2(x, y);

            // If other is not a patch to be sampled, skip it
            if (pos[other_c].a < 1.0f || (useSubstructuring && lig_in[other_c].a < 1.0f)) continue;


            // Viscaching and randomization are mutually exclusive.
            #if VISCACHE

            // Use viscache beyond the first pass
            if (passNum > 0) {
                // Get viscache
                uint bufPos = getBufferPos(self_c, other_c, texRes);

                if (bufPos <= max_bufferpos) {
                    if (getVisible(bufPos, vis)) {
                        setColor(self_c, other_c);
                    }
                    continue;
                }
                continue;
            }

            #elif RANDOMIZE

            // Sample random patch is sampling resolution
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


            // Do not sample ourselves
            if (other_c.x - self_c.x < sampling_res && other_c.y - self_c.y < sampling_res) continue;

            // World position of other
            float3 other_wpos = pos[other_c].xyz + minPos;


            #if VOXELRAYMARCH

            // Voxel raymarch
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

            TraceRay(gScene.rtAccel,                    // A Falcor built-in containing the raytracing acceleration structure
                RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH | RAY_FLAG_SKIP_CLOSEST_HIT_SHADER,  // Ray flags.  (Here, we will skip hits with back-facing triangles)
                0xFF,                                   // Instance inclusion mask.  0xFF => no instances discarded from this mask
                0,                                      // Hit group to index (i.e., when intersecting, call hit shader #0)
                0,                                      // Number of hit groups ('hitProgramCount' is built-in from Falcor with the right number)
                0,                                      // Miss program index
                ray,                                    // Data structure describing the ray to trace
                rpl
            );
        }
    }

    #endif
}


// MISS SHADER
[shader("miss")]
void primaryMiss(inout RayPayload rpl)
{
    #if HEMISPHERIC

    // Do nothing on hemispheric sampling

    #else

    // In classic sampling, apply the lighting contribution

    uint2 self_c = rpl.self_c;
    uint2 other_c = rpl.other_c;

    #if VISCACHE
    uint bufPos = getBufferPos(self_c, other_c, texRes);
    if (bufPos <= max_bufferpos) {
        setVisible(bufPos, vis);
    }
    #endif

    setColor(self_c, other_c);

    #endif
}


// Applies color ontribution from other to self (classic sampling)
void setColor(uint2 self_c, uint2 other_c) {
    // World positions
    float3 self_wpos = pos[self_c].xyz + minPos;
    float3 other_wpos = pos[other_c].xyz + minPos;

    // Distance
    float3 self_to_other = other_wpos - self_wpos;
    float r = length(self_to_other) * distance_factor;

    // Cosines
    self_to_other = normalize(self_to_other);
    float3 self_nrm = nrm[self_c].xyz;
    float3 other_nrm = nrm[other_c].xyz;
    float self_cos = dot(self_nrm, self_to_other);
    float other_cos = dot(other_nrm, -self_to_other);

    if (self_cos <= 0.0f || other_cos <= 0.0f) return;

    // Form factor
    float F = self_cos * other_cos * (1.0f / (PI * r * r));
    F = min(1.0f, F); // Limiting view factor to 1.0f

    // Surface area
    float other_surface = arf[other_c].r; // surface area of other

    // Color of self
    float4 self_color = float4(mat[self_c].rgb, 1.0f);

    // Radiance of other
    #if MIPMAPPED_UNDERSAMPLING
    float ha = float(sampling_res) * 0.5f;
    float2 uvs = float2(float(other_c.x)+ha, float(other_c.y)+ha) / float2(float(texRes), float(texRes));
    float4 R = lig_in.SampleLevel(sampleWrap, uvs, log2(sampling_res));
    #else
    float4 R = lig_in[other_c];
    #endif

    // Apply contribution
    lig_out[self_c] += (sampling_res * sampling_res) * (lig_in[other_c].a * R * self_color * reflectivity_factor * F * other_surface);
    lig_out[self_c].a = 1.0f;
}


// CLOSEST HIT
// Applies color contribution from hitpoint to rpl.self_c in hemispheric sampling
[shader("closesthit")]
void primaryClosestHit(inout RayPayload rpl, in BuiltInTriangleIntersectionAttributes attribs)
{
    #if HEMISPHERIC

    // Origin
    uint2 self_c = rpl.self_c;

    // Destination
    VertexData v = getVertexData(getGeometryInstanceID(), PrimitiveIndex(), attribs);
    float2 other_uv = v.texC;

    // Use this to create an image of which patches are sampled:
    //if (self_c.x == 4 && self_c.y == 4) {
    //    lig_out[self_c] = float4(1, 0, 0, 1);
    //    lig_out[other_uv * uint2(texRes, texRes)] = float4(1, 1, 1, 1);
    //}
    //return;

    // World position
    float3 self_wpos = pos[self_c].xyz + minPos;
    float3 other_wpos = pos.SampleLevel(sampleWrap, other_uv, 0).xyz + minPos;

    // Distance
    float3 self_to_other = other_wpos - self_wpos;
    float r = length(self_to_other) * distance_factor;

    if (r < 0.1f) return;

    // Cosine
    self_to_other = normalize(self_to_other);
    float3 self_nrm = nrm[self_c].xyz;
    float self_cos = dot(self_nrm, self_to_other);
    if (self_cos <= 0.0f) return;

    // Form factor
    float view_factor = self_cos * (1.0f / (PI * max(r * r, 0.1f)));

    // Color of self
    float4 self_color = float4(mat[rpl.self_c].rgb, 1.0f);

    // Radiance of other
    float4 R = lig_in.SampleLevel(sampleWrap, other_uv, 1);

    // Apply contribution
    lig_out[self_c] += (R * self_color * reflectivity_factor * view_factor) / float(hemisphere_samples);

    #endif
}
