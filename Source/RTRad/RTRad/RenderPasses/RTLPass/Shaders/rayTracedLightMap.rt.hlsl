import Scene.Raytracing;
import Scene.Shading;
import Utils.Sampling.TinyUniformSampleGenerator;
import Experimental.Scene.Lights.LightHelpers;
import Experimental.Scene.Material.StandardMaterial;

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
    float3 posOffset;
    int passNum;
    bool useVisCache;
    int texRes;

    bool randomizeSamples;
    bool useSubstructuring;
};

SamplerState sampleWrap : register(s0);

#define PI 3.14159265359f
#define max_bufferpos 1073676288

struct RayPayload
{
    uint2 self_c;
    uint2 other_c;
};

uint3 toVoxelSpace(float3 posW) {
    posW = 0.5f * (posW + float3(1, 1, 1)); // [0,1]
    posW = posW * 63.49f; // [0, 64]
    return (uint3)posW;
}

float max3(float3 v) {
    return max(v.x, max(v.y, v.z));
}

bool rayMarchVisible(float3 self_pos, float3 other_pos)
{
    //epsilon
    self_pos = self_pos + normalize(other_pos - self_pos) * 0.07f;
    other_pos = other_pos + normalize(self_pos - other_pos) * 0.07f;

    for (float i = 0.0f; i < 1.0f; i += 1.0f / 128.0f) {
        float3 pos = self_pos + i * (other_pos - self_pos);

        if (max3(abs(pos)) > 1.0f) return true;

        uint3 voxPos = toVoxelSpace(pos);

        if (voxTex[voxPos].a > 0) {
            return false;
        }
    }
    return true;
}

bool rayMarchVisible(uint2 self_c, uint2 other_c)
{
    float3 self_pos = pos[self_c].xyz;
    float3 other_pos = pos[other_c].xyz;

    //epsilon
    self_pos = self_pos + normalize(other_pos - self_pos) * 0.07f;
    other_pos = other_pos + normalize(self_pos - other_pos) * 0.07f;

    //float delta = 1.0f / (length(self_pos - other_pos) * 32.0f);

    for (float i = 0.0f; i < 1.0f; i += 1.0f / 32.0f) {
        float3 pos = self_pos + i * (other_pos - self_pos);

        if (max3(abs(pos)) > 1.0f) return true;

        uint3 voxPos = toVoxelSpace(pos);

        if (voxTex[voxPos].a > 0) {
            return false;
        }
    }
    return true;
}

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

uint cantor(uint x, uint y)
{
    if (x > y) {
        uint tmp = x;
        x = y;
        y = tmp;
    }

    x = texRes - x;
    return (x + y)* (x + y + 1) / 2 + x;
}

uint getBufferPos(uint2 self_c, uint2 other_c)
{
    uint self_p = self_c.x + self_c.y * texRes;
    uint other_p = other_c.x + other_c.y * texRes;

    return cantor(self_p, other_p);
}

bool getVisible(uint bufferpos) {
    uint bitpos = bufferpos % 32;

    bufferpos = bufferpos / 32;

    uint v = vis[bufferpos];

    return ((v >> bitpos) & 1) > 0;
}

void setVisible(uint bufferpos) {
    uint bitpos = bufferpos % 32;

    bufferpos = bufferpos / 32;

    uint v = vis[bufferpos];

    vis[bufferpos] = v | (1 << bitpos);
}

float distSquared(float3 pos1, float3 pos2) {
    float3 c = pos1 - pos2;
    return dot(c, c);
}

[shader("raygeneration")]
void rayGen()
{
    uint2 self_c = DispatchRaysIndex().xy + currentOffset;

    //self_c.x += row_offset;

    // If pos alpha is less than 1, skip this.
    if (pos[self_c].a < 0.1f) {
        return;
    }

    // Worls position of current texel
    float3 self_wpos = pos[self_c].xyz + posOffset;

    float dim1;
    float dim2;
    pos.GetDimensions(dim1, dim2);

    float4 sum_c = float4(0, 0, 0, 0);

    uint matID = (uint) mat[self_c].a;
    float3 emissive = gScene.materials[matID].emissive;

    lig2[self_c] = float4(emissive, 1.f);

    // Lets not run this for light-sources
    // TODO: renable. I turned it off for bugfixing
    //if (length(emissive) > 0.1f) return;

    
    //lig2[self_c] = lig[self_c];

    // Use this to display a cool way of what points rayMarch determines to be visible from a given point
    /*lig2[self_c] = float4(0, 0, 0, 1);
    if (rayMarchVisible(self_wpos, float3(0.f, 0.f, -1.0f))) {
        lig2[self_c] = float4(1,1,1,1);
    }
    return;*/

    bool voxelRayMarch = false;
    //voxelRayMarch = self_c.x & 4 > 0;

    for (uint x = 0; x < dim1; x += sampling_res) {
        for (uint y = 0; y < dim2; y += sampling_res) {

            uint2 other_c = uint2(x, y);

            if (pos[other_c].a < 1.0f || (useSubstructuring && lig[other_c].a < 1.0f)) continue;

            if (self_c.x == other_c.x && self_c.y == other_c.y) continue;

            if (useVisCache && passNum > 0) {
                // Get viscache
                uint bufPos = getBufferPos(self_c, other_c);

                if (bufPos <= max_bufferpos) {
                    if (getVisible(bufPos)) {
                        setColor(self_c, other_c);
                    }
                    continue;
                }
            }

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

            float3 other_wpos = pos[other_c].xyz + posOffset;

            //bool voxelRayMarch = distSquared(other_wpos, self_wpos) > 4.6f;//length(other_wpos - self_wpos) > 1.8f;
            //bool voxelRayMarch = false;

            if (voxelRayMarch) {
                if (rayMarchVisible(self_c, other_c)) {
                    setColor(self_c, other_c);
                }
            }
            else {
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
}

[shader("miss")]
void primaryMiss(inout RayPayload rpl)
{
    uint2 self_c = rpl.self_c;
    uint2 other_c = rpl.other_c;

    // Set visCache
    //uint bufPos = getBufferPos(self_c, other_c);

    //if (vis[bufPos] != 100) lig2[self_c] += float4(0.01, 0, 0, 0);

    //vis[bufPos] = 100;

    //bufPos = getBufferPos(other_c, self_c);

    //vis[bufPos] = 100;

    if (useVisCache) {
        uint bufPos = getBufferPos(self_c, other_c);
        if (bufPos <= max_bufferpos) {
            setVisible(bufPos);
        }
    }
    //setVisible(other_c, self_c);


    setColor(self_c, other_c);
}

#define ref 0.9f

void setColor(uint2 self_c, uint2 other_c) {
    float3 self_wpos = pos[self_c].xyz + posOffset;// (2.0f * pos[self_c]).xyz - float3(1.f, 1.f, 1.f);
    float3 other_wpos = pos[other_c].xyz + posOffset;// (2.0f * pos[other_c]).xyz - float3(1.f, 1.f, 1.f);

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
