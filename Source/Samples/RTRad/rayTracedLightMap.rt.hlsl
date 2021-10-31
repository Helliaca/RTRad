import Scene.Raytracing;
import Scene.Shading;
import Utils.Sampling.TinyUniformSampleGenerator;
import Experimental.Scene.Lights.LightHelpers;
import Experimental.Scene.Material.StandardMaterial;

Texture2D<float4> pos;
Texture2D<float4> nrm;
Texture2D<float> arf;
Texture2D<float4> lig;
RWTexture2D<float4> lig2;

cbuffer PerFrameCB {
    uint2 last_index;
};

#define PI 3.14159265359f

struct RayPayload
{
    uint2 self_c;
    uint2 other_c;
};

[shader("raygeneration")]
void rayGen()
{
    uint2  self_c = DispatchRaysIndex().xy;

    self_c += last_index;

    // If pos alpha is less than 1, skip this.
    if (pos[self_c].a < 1.f) return;

    // Worls position of current texel
    float4 self_wpos = (2.0f * pos[self_c]) - float4(1.f, 1.f, 1.f, 1.f);

    float dim1;
    float dim2;
    pos.GetDimensions(dim1, dim2);

    float4 sum_c = float4(0, 0, 0, 0);

    lig2[self_c] = float4(0.f, 0.f, 0.f, 1.f);
    if (self_wpos.y > 0.99f) {
        lig2[self_c] = float4(1.f, 1.f, 1.f, 1.f);
    }

    for (uint x = 0; x < dim1; x += 1) {
        for (uint y = 0; y < dim2; y += 1) {

            uint2 other_c = uint2(x, y);

            float4 other_wpos = (2.0f * pos[other_c]) - float4(1.f, 1.f, 1.f, 1.f);

            RayDesc ray;
            ray.Origin = self_wpos.xyz;
            ray.Direction = normalize(other_wpos.xyz - self_wpos.xyz);
            ray.TMin = 0.01f;
            ray.TMax = distance(self_wpos.xyz, other_wpos.xyz) - (2.0f * ray.TMin);

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

    float3 self_wpos = (2.0f * pos[self_c]).xyz - float3(1.f, 1.f, 1.f);
    float3 other_wpos = (2.0f * pos[other_c]).xyz - float3(1.f, 1.f, 1.f);

    float3 self_to_other = other_wpos - self_wpos;

    float r = length(self_to_other);

    // Avoiding self-illuimination
    if (r < 0.05f) return;
    if (self_c.x == other_c.x && self_c.y == other_c.y) return;

    self_to_other = normalize(self_to_other);

    float3 self_nrm = 2.0f * (nrm[self_c].xyz - 0.5f);
    float3 other_nrm = 2.0f * (nrm[other_c].xyz - 0.5f);

    float self_cos = dot(self_nrm, self_to_other);
    float other_cos = dot(other_nrm, -self_to_other);

    if (self_cos <= 0.0 || other_cos <= 0.0) return;

    float view_factor = self_cos * other_cos * (1.0f / (PI * r * r));

    float ref = 0.9;

    // Account for surface area of other
    float dim1;
    float dim2;
    pos.GetDimensions(dim1, dim2);
    float fpa = (dim1 * dim2) * arf[other_c].r; // -> fragments per unit area on other

    float4 self_color = float4(1.f, 1.f, 1.f, 1.f);
    if (self_wpos.x > 0.99f) self_color = float4(1.f, 0.f, 0.f, 1.f);
    if (self_wpos.x < -0.99f) self_color = float4(0.f, 1.f, 0.f, 1.f);

    //rpl.added_c = (lig[other_c] / fpa) * self_color * ref * view_factor;

    lig2[self_c] += (lig[other_c] / fpa) * self_color * ref * view_factor;
}
