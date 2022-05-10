import Scene.Raster;
import Scene.Shading;
import Utils.Sampling.TinyUniformSampleGenerator;
import Experimental.Scene.Lights.LightHelpers;
import Experimental.Scene.Material.StandardMaterial;

RWTexture2D<float4> lig;
Texture2D<float4> pos;
Texture2D<float4> nrm;

cbuffer PerFrameCB {
    int step;
};

struct GsOut
{
    float2 texC : TEXCOORD;
};

float get_gradient(uint2 uv, int s) {
    // mean color of siblings
    float3 mean = 0.25f * (
        lig[uv + uint2(0, 0)].rgb +
        lig[uv + uint2(s, 0)].rgb +
        lig[uv + uint2(0, s)].rgb +
        lig[uv + uint2(s, s)].rgb
    );
    return length(lig[uv].rgb - mean);
}

float4 main(GsOut vsOut) : SV_TARGET0
{
    // Get texture dimensions
    float xres, yres;
    lig.GetDimensions(xres, yres);

    // Get tex-coords of this fragment
    uint2 uv = (uint2)(vsOut.texC * float2(xres, yres));

    // Make sure we run for the correct pixels
    if (uv.x % step != 0 || uv.y % step != 0) return pos[uv];

    /*int parent_step = 2 * step;
    uint2 parent = uv - (uv % parent_step);*/

    if (step == 1) {
        lig[uv].a = 1.0f;
        return pos[uv];
    }

    // look at the 4 kiddos of node
    int s = step/2;
    uint2 c0 = uv + uint2(0, 0);
    uint2 c1 = uv + uint2(0, s);
    uint2 c2 = uv + uint2(s, 0);
    uint2 c3 = uv + uint2(s, s);

    // if ANY of them have a smaller number than step*step, do nothing
    bool stuff = false;
    if (lig[c0].a < s * s) stuff = true;
    if (lig[c1].a < s * s) stuff = true;
    if (lig[c2].a < s * s) stuff = true;
    if (lig[c3].a < s * s) stuff = true;
    if(stuff) return pos[uv];

    // otherwise:
    float3 mean = 0.25f * (lig[c0].rgb + lig[c1].rgb + lig[c2].rgb + lig[c3].rgb);
    float gradient = length(mean - lig[c0].rgb);

    if (gradient < 0.1f) {
        lig[c0].a = lig[c0].a + lig[c1].a + lig[c2].a + lig[c3].a;
        lig[c1].a = 0.0f;
        lig[c2].a = 0.0f;
        lig[c3].a = 0.0f;
    }



    return pos[uv];
}
