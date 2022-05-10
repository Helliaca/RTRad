import Scene.Raster;
import Scene.Shading;
import Utils.Sampling.TinyUniformSampleGenerator;
import Experimental.Scene.Lights.LightHelpers;
import Experimental.Scene.Material.StandardMaterial;

Texture2D<float4> lig;
Texture2D<float4> pos;
Texture2D<float4> nrm;

cbuffer PerFrameCB {
    int step;
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

    if (uv.x % step != 0 || uv.y % step != 0) return lig[uv];

    // Get gradient
    float3 mean = float3(0, 0, 0);
    for (int x = 0; x < step; x++) {
        for (int y = 0; y < step; y++) {
            mean += lig[uv + uint2(x, y)].rgb;
        }
    }
    mean = mean / (step * step);

    // Variance
    float var = length(mean - lig[uv].rgb);


    // do things


    /*float dim1;
    float dim2;
    ligTex.GetDimensions(dim1, dim2);

    int2 c = int2(vsOut.texC.x * dim1, vsOut.texC.y * dim2);

    int max_radius = 3;

    float min_dist = 999999.f;
    int2 min_c = c;*/

    return float4(var,0,0,1);
}
