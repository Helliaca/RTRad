import Scene.Raytracing;
import Scene.Shading;
import Utils.Sampling.TinyUniformSampleGenerator;
import Experimental.Scene.Lights.LightHelpers;
import Experimental.Scene.Material.StandardMaterial;
import RTRad.HemisphericSampling;


Texture2D<float4> pos;
Texture2D<float4> nrm;
Texture2D<float> arf;
Texture2D<float4> mat;
Texture2D<float4> lig;
RWTexture2D<float4> lig2;

Texture3D<float4> voxTex;

cbuffer PerFrameCB {
    uint2 currentOffset;

    int sampling_res;
    float3 posOffset;
    bool randomizeSamples;
    int passNum;
    bool useVisCache;
    int texRes;
};

SamplerState sampleWrap : register(s0);

#define PI 3.14159265359f

struct RayPayload
{
    uint2 self_c;
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

float3 perp(float3 v, float3 q) {
    v = normalize(v);
    q = normalize(q);
    return cross(v, q);
}

float3 toForward(float3 i, float3 forward) {
    float3 rv = float3(1.f, 0.f, 0.f);

    float3 y = normalize(forward);

    // This is here so we dont have rv==y
    if(y.x>0.99f) rv = float3(0.f, 0.f, 1.f);

    float3 x = -perp(y, rv);
    float3 z = -perp(y, x);
    float3x3 mat = {
        x, y, z
    };
    return mul(i, mat);
}

// Utility function to get a vector perpendicular to an input vector 
//    (from "Efficient Construction of Perpendicular Vectors Without Branching")
float3 getPerpendicularVector(float3 u)
{
    float3 a = abs(u);
    uint xm = ((a.x - a.y) < 0 && (a.x - a.z) < 0) ? 1 : 0;
    uint ym = (a.y - a.z) < 0 ? (1 ^ xm) : 0;
    uint zm = 1 ^ (xm | ym);
    return cross(u, float3(xm, ym, zm));
}

float3 perp(float3 u)
{
    float3 a = abs(u);
    uint xm = ((a.x - a.y) < 0 && (a.x - a.z) < 0) ? 1 : 0;
    uint ym = (a.y - a.z) < 0 ? (1 ^ xm) : 0;
    uint zm = 1 ^ (xm | ym);
    return cross(u, float3(xm, ym, zm));
}

// Generates a seed for a random number generator from 2 inputs plus a backoff
uint initRand(uint val0, uint val1, uint backoff = 16)
{
    uint v0 = val0, v1 = val1, s0 = 0;

    [unroll]
    for (uint n = 0; n < backoff; n++)
    {
        s0 += 0x9e3779b9;
        v0 += ((v1 << 4) + 0xa341316c) ^ (v1 + s0) ^ ((v1 >> 5) + 0xc8013ea4);
        v1 += ((v0 << 4) + 0xad90777d) ^ (v0 + s0) ^ ((v0 >> 5) + 0x7e95761e);
    }
    return v0;
}

// Takes our seed, updates it, and returns a pseudorandom float in [0..1]
float nextRand(inout uint s)
{
    s = (1664525u * s + 1013904223u);
    return float(s & 0x00FFFFFF) / float(0x01000000);
}

// Source: https://github.com/NVIDIAGameWorks/GettingStartedWithRTXRayTracing/blob/master/11-OneShadowRayPerPixel/Data/Tutorial11/diffusePlus1ShadowUtils.hlsli
// Get a cosine-weighted random vector centered around a specified normal direction.
float3 getCosHemisphereSample(inout uint randSeed, float3 hitNorm)
{
    // Get 2 random numbers to select our sample with
    float2 randVal = float2(nextRand(randSeed), nextRand(randSeed));

    // Cosine weighted hemisphere sample from RNG
    float3 bitangent = getPerpendicularVector(hitNorm);
    float3 tangent = cross(bitangent, hitNorm);
    float r = sqrt(randVal.x);
    float phi = 2.0f * 3.14159265f * randVal.y;

    // Get our cosine-weighted hemisphere lobe sample direction
    return tangent * (r * cos(phi).x) + bitangent * (r * sin(phi)) + hitNorm.xyz * sqrt(1 - randVal.x);
}

float3 toTangentSpace(float3 vec, float3 nrm) {
    float3 bitangent = perp(nrm);
    float3 tangent = cross(bitangent, nrm);

    float3x3 m = {
        tangent,
        bitangent,
        nrm
    };

    return mul(m, vec);
}

#define samples 100

[shader("raygeneration")]
void rayGen()
{
    uint2 self_c = DispatchRaysIndex().xy + currentOffset;

    // If pos alpha is less than 1, skip this.
    if (pos[self_c].a < 1.f) {
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
    if (length(emissive) > 0.1f) return;

    float3 surface_normal = 2.0f * (nrm[self_c].xyz - 0.5f);

    uint seed = initRand(passNum, passNum + 4124512);

    for (int i = 0; i < samples; i++) {
        //float3 rv = getCosHemisphereSample(seed, surface_normal);
        float3 rv = sampledirs[i];
        rv = toTangentSpace(rv, surface_normal);
        make_ray(self_wpos, rv, self_c);
    }
}

void make_ray(float3 self_wpos, float3 dirv, uint2 self_c) {
    float3 surface_normal = 2.0f * (nrm[self_c].xyz - 0.5f);

    RayDesc ray;
    ray.Origin = self_wpos;

    //ray.Direction = normalize(surface_normal);
    dirv = toForward(dirv, surface_normal);
    ray.Direction = dirv;

    ray.TMin = 0.001f;
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

[shader("miss")]
void primaryMiss(inout RayPayload rpl)
{
    
}

[shader("closesthit")]
void primaryClosestHit(inout RayPayload rpl, in BuiltInTriangleIntersectionAttributes attribs)
{
    VertexData v = getVertexData(getGeometryInstanceID(), PrimitiveIndex(), attribs);

    uint2 self_c = rpl.self_c;

    float2 uv = v.texC;

    float3 self_wpos = pos[self_c].xyz + posOffset;
    float3 other_wpos = pos.SampleLevel(sampleWrap, uv, 0).xyz + posOffset;

    float3 self_to_other = other_wpos - self_wpos;

    float r = length(self_to_other);

    self_to_other = normalize(self_to_other);

    float3 self_nrm = 2.0f * (nrm[self_c].xyz - 0.5f);
    float3 other_nrm = 2.0f * (nrm.SampleLevel(sampleWrap, uv, 0).xyz - 0.5f);

    float self_cos = dot(self_nrm, self_to_other);
    float other_cos = dot(other_nrm, -self_to_other);

    float view_factor = self_cos * other_cos * (1.0f / (PI * r * r));

    float4 col = lig.SampleLevel(sampleWrap, uv, 0);

    float4 self_color = float4(mat[rpl.self_c].rgb, 1.0f);

    float ref = 0.9;

    lig2[self_c] += (col / samples) * self_color * ref * view_factor;
}

[shader("anyhit")]
void primaryAnyHit(inout RayPayload rpl, BuiltInTriangleIntersectionAttributes attribs)
{
    
}
