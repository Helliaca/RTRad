import Scene.Raster;
import Scene.Shading;
import Utils.Sampling.TinyUniformSampleGenerator;
import Experimental.Scene.Lights.LightHelpers;
import Experimental.Scene.Material.StandardMaterial;

RWTexture2D<float4> lig;

cbuffer PerFrameCB {
    
};

struct GsOut
{
    float2 texC : TEXCOORD;
};

float4 main(GsOut vsOut) : SV_TARGET0
{
    // Get texture dimensions
    float xres, yres;
    lig.GetDimensions(xres, yres);

    // Get tex-coords of this fragment
    uint2 uv = (uint2)(vsOut.texC * float2(xres, yres));

    lig[uv] = float4(1, 1, 1, 1);

    return lig[uv];
}
