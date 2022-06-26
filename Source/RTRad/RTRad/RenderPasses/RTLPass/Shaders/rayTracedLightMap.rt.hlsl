import Scene.Raytracing;
import Scene.Shading;
import Utils.Sampling.TinyUniformSampleGenerator;
import Experimental.Scene.Lights.LightHelpers;
import Experimental.Scene.Material.StandardMaterial;
import RTRad.RTRad.Slang.VisCaching;
import RTRad.RTRad.Slang.Voxel;

Texture2D<float4> pos;
Texture2D<float4> nrm;
Texture2D<float> arf;
Texture2D<float4> mat;
Texture2D<float4> lig;

Texture3D<float4> voxTex;

RWTexture2D<float4> lig2;

RWBuffer<uint> vis : register(t9);

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

#define PI 3.14159265359f
#define max_bufferpos 4294705152

struct RayPayload
{
    uint2 self_c;
    uint2 other_c;
};

// Hash function from H. Schechter & R. Bridson, goo.gl/RXiKaH
uint Hash(uint seed)
{
    seed = (seed ^ 61) ^ (seed >> 16);
    seed *= 9;
    seed = seed ^ (seed >> 4);
    seed *= 0x27d4eb2d;
    seed = seed ^ (seed >> 15);
    return seed;
}

uint random(uint seed, uint max)
{
    return Hash(seed) % max;
}

uint2 random(uint2 seed, uint2 max)
{
    return uint2(
        Hash(seed.x) % max.x,
        Hash(seed.y) % max.y
    );
}

float distSquared(float3 pos1, float3 pos2) {
    float3 c = pos1 - pos2;
    return dot(c, c);
}

[shader("raygeneration")]
void rayGen()
{
    uint2 self_c = DispatchRaysIndex().xy + currentOffset;

    // If pos alpha is less than 1, skip this.
    //if (pos[self_c].a < 1.0f) {
    //    return;
    //}

    // Worls position of current texel
    float3 self_wpos = pos[self_c].xyz + minPose;

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

    for (uint x = 0; x < dim1; x += sampling_res) {
        for (uint y = 0; y < dim2; y += sampling_res) {
            uint2 other_c = uint2(x, y);

            if (pos[other_c].a < 1.0f || (useSubstructuring && lig[other_c].a < 1.0f)) continue;

            if (self_c.x == other_c.x && self_c.y == other_c.y) continue;

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
            #endif

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

            float3 other_wpos = pos[other_c].xyz + minPose;

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

#define ref 0.9f

void setColor(uint2 self_c, uint2 other_c) {
    float3 self_wpos = pos[self_c].xyz + minPose;// (2.0f * pos[self_c]).xyz - float3(1.f, 1.f, 1.f);
    float3 other_wpos = pos[other_c].xyz + minPose;// (2.0f * pos[other_c]).xyz - float3(1.f, 1.f, 1.f);

    float3 self_to_other = other_wpos - self_wpos;

    float r = length(self_to_other);

    // Avoiding self-illuimination
    //if (r < 0.1f) return;
    if (abs(self_c.x - other_c.x) < sampling_res && abs(self_c.y - other_c.y) < sampling_res) return;

    // Form factor
    self_to_other = normalize(self_to_other);

    float3 self_nrm = 2.0f * (nrm[self_c].xyz - 0.5f);
    float3 other_nrm = 2.0f * (nrm[other_c].xyz - 0.5f);

    float self_cos = dot(self_nrm, self_to_other);
    float other_cos = dot(other_nrm, -self_to_other);

    if (self_cos <= 0.0f || other_cos <= 0.0f) return;

    float F = self_cos * other_cos * (1.0f / (PI * r * r));

    // Lighting
    float other_surface = arf[other_c].r; // surface area of other

    float4 self_color = float4(mat[self_c].rgb, 1.0f);

    float dim1;
    float dim2;
    pos.GetDimensions(dim1, dim2);
    float2 uvs = float2(float(other_c.x), float(other_c.y)) / float2(float(dim1), float(dim2));

    float4 other_lig = lig.SampleLevel(sampleWrap, uvs, log2(sampling_res));

    lig2[self_c] += (sampling_res * sampling_res) * (lig[other_c].a * other_lig * self_color * ref * F * other_surface);
    lig2[self_c].a = 1.0f;
}
