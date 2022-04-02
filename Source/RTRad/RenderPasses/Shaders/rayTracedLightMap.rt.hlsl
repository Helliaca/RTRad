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
RWTexture2D<float4> lig2;

RWBuffer<uint> vis : register(t9);

cbuffer PerFrameCB {
    int row_offset;
    int sampling_res;
    float3 posOffset;
    bool randomizeSamples;
    int passNum;
    bool useVisCache;
    int texRes;
};

SamplerState sampleWrap : register(s0);

#define PI 3.14159265359f
#define max_bufferpos 1073676288

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

[shader("raygeneration")]
void rayGen()
{
    uint2 self_c = DispatchRaysIndex().xy;

    self_c.x += row_offset;

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


    for (uint x = 0; x < dim1; x += sampling_res) {
        for (uint y = 0; y < dim2; y += sampling_res) {

            uint2 other_c = uint2(x, y);

            if (self_c.x == other_c.x && self_c.y == other_c.y) continue;

            if (useVisCache) {
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
                    random((self_c.x + 1) * (self_c.y + 1) + passNum, 7864128),
                    random((self_c.x + 1) * (self_c.y + 1) + passNum, 5490141)
                    );

                uint2 rnd = random(seed, uint2(sampling_res, sampling_res));

                other_c += uint2(
                    rnd.x,
                    rnd.y
                    );
            }

            float3 other_wpos = pos[other_c].xyz + posOffset;

            RayDesc ray;
            ray.Origin = self_wpos;
            ray.Direction = normalize(other_wpos - self_wpos);
            ray.TMin = 0.0001f;
            ray.TMax = distance(self_wpos, other_wpos) - (2.0f * ray.TMin);

            RayPayload rpl = { self_c, other_c };

            TraceRay(gScene.rtAccel,                        // A Falcor built-in containing the raytracing acceleration structure
                RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH,  // Ray flags.  (Here, we will skip hits with back-facing triangles)
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

    // Set visCache
    //uint bufPos = getBufferPos(self_c, other_c);

    //if (vis[bufPos] != 100) lig2[self_c] += float4(0.01, 0, 0, 0);

    //vis[bufPos] = 100;

    //bufPos = getBufferPos(other_c, self_c);

    //vis[bufPos] = 100;

    uint bufPos = getBufferPos(self_c, other_c);
    if (bufPos <= max_bufferpos) {
        setVisible(bufPos);
    }
    //setVisible(other_c, self_c);


    setColor(self_c, other_c);
}

void setColor(uint2 self_c, uint2 other_c) {
    float3 self_wpos = pos[self_c].xyz + posOffset;// (2.0f * pos[self_c]).xyz - float3(1.f, 1.f, 1.f);
    float3 other_wpos = pos[other_c].xyz + posOffset;// (2.0f * pos[other_c]).xyz - float3(1.f, 1.f, 1.f);

    float3 self_to_other = other_wpos - self_wpos;

    float r = length(self_to_other);

    // Avoiding self-illuimination
    //if (r < 0.1f) return;
    if (abs(self_c.x - other_c.x) < sampling_res && abs(self_c.y - other_c.y) < sampling_res) return;

    self_to_other = normalize(self_to_other);

    float3 self_nrm = 2.0f * (nrm[self_c].xyz - 0.5f);
    float3 other_nrm = 2.0f * (nrm[other_c].xyz - 0.5f);

    float self_cos = dot(self_nrm, self_to_other);
    float other_cos = dot(other_nrm, -self_to_other);

    if (self_cos <= 0.0f || other_cos <= 0.0f) return;

    float view_factor = self_cos * other_cos * (1.0f / (PI * r * r));

    float ref = 0.9;

    // Account for surface area of other
    float dim1;
    float dim2;
    pos.GetDimensions(dim1, dim2);
    float fpa = (dim1 * dim2) / (sampling_res * sampling_res) * arf[other_c].r; // -> fragments per unit area on other

    float4 self_color = float4(mat[self_c].rgb, 1.0f);

    // old version (where we stored a matid in mat.r)
    //uint matID = (uint) mat[self_c].r;
    //float4 self_color = gScene.materials[matID].baseColor;

    float2 uvs = float2(float(other_c.x), float(other_c.y)) / float2(float(dim1), float(dim2));

    float4 col = lig.SampleLevel(sampleWrap, uvs, log2(sampling_res));

    lig2[self_c] += (col / fpa) * self_color * ref * view_factor;

    //lig2[self_c] += (lig[other_c] / fpa) * self_color * ref * view_factor;
}
