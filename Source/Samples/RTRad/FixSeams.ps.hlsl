// Falcor / Slang imports to include shared code and data structures
//__import Shading;           // Imports ShaderCommon and DefaultVS, plus material evaluation
//__import DefaultVS;         // VertexOut declaration

import Scene.Raster;
import Scene.Shading;
import Utils.Sampling.TinyUniformSampleGenerator;
import Experimental.Scene.Lights.LightHelpers;
import Experimental.Scene.Material.StandardMaterial;

Texture2D<float4> ligTex;

struct GsOut
{
    float2 texC : TEXCOORD;
};

float4 main(GsOut vsOut) : SV_TARGET0
{
    float dim1;
    float dim2;
    ligTex.GetDimensions(dim1, dim2);

    int2 c = int2(vsOut.texC.x * dim1, vsOut.texC.y * dim2);

    int max_radius = 3;

    float min_dist = 999999.f;
    int2 min_c = c;

    if (ligTex[c].a < 1.0f) {

        for (int x = -max_radius/2; x < max_radius/2; x++) {
            for (int y = -max_radius/2; y < max_radius/2; y++) {
                int2 coord = c + int2(x, y);
                coord = max(coord, int2(0, 0));
                coord = min(coord, int2(dim1, dim2));

                if (ligTex[coord].a > 0.1f && distance(coord, c) < min_dist) {
                    min_c = coord;
                    min_dist = distance(coord, c);
                }
            }
        }

        return ligTex[min_c];
    };

    return ligTex[c];
}


