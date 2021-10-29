// Falcor / Slang imports to include shared code and data structures
//__import Shading;           // Imports ShaderCommon and DefaultVS, plus material evaluation
//__import DefaultVS;         // VertexOut declaration

import Scene.Raster;
import Scene.Shading;
import Utils.Sampling.TinyUniformSampleGenerator;
import Experimental.Scene.Lights.LightHelpers;
import Experimental.Scene.Material.StandardMaterial;

//RWTexture2D<float4> posTex;

struct GBuffer
{
    float4 pos    : SV_Target0;
};


GBuffer pmain(VSOut vsOut, uint triangleIndex : SV_PrimitiveID)
{
    GBuffer o;
    o.pos = float4(vsOut.posW, 1.f);
    return o;
}


