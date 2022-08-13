import Scene.Raster;
import Scene.Shading;
import Utils.Sampling.TinyUniformSampleGenerator;
import Experimental.Scene.Lights.LightHelpers;
import Experimental.Scene.Material.StandardMaterial;

Texture2D<float4> pos;
RWTexture2D<float4> lig;

cbuffer PerFrameCB {
    
};

struct GsOut
{
    float2 texC : TEXCOORD;
};

static const uint2 nes[8] = {
    uint2(1,0),
    uint2(0,1),
    uint2(-1,0),
    uint2(0,-1),
    uint2(1,1),
    uint2(-1,1),
    uint2(1,-1),
    uint2(-1,-1)
};

float4 main(GsOut vsOut) : SV_TARGET0
{
    // Get texture dimensions
    float xres, yres;
    lig.GetDimensions(xres, yres);

    // Get tex-coords of this fragment
    uint2 uv = (uint2)(vsOut.texC * float2(xres, yres));

    // If this is not an "actual patch", then look for a patch in its neighbours and assume the its value.
    if (pos[uv].a < 0.1f) {
        float3 o = lig[uv].rgb;
        float l = 999.0f;

        for (int i = 0; i < 8; i++) {
            // We prioritize lower values, to prevent overly bright spots
            if (pos[uv + nes[i]].a > 0.9f && length(lig[uv + nes[i]].rgb) < l) {
                o = lig[uv + nes[i]].rgb;
                l = length(o);
            }
        }

        lig[uv].rgb = o.rgb;
    }

    return lig[uv];
}
