// Falcor / Slang imports to include shared code and data structures
//__import Shading;           // Imports ShaderCommon and DefaultVS, plus material evaluation
//__import DefaultVS;         // VertexOut declaration

import Scene.Raster;
import Scene.Shading;
import Utils.Sampling.TinyUniformSampleGenerator;
import Experimental.Scene.Lights.LightHelpers;
import Experimental.Scene.Material.StandardMaterial;

Texture2D<float4> disTex;

SamplerState sampleWrap : register(s0);


float4 pmain(VSOut vsOut, uint triangleIndex : SV_PrimitiveID) : SV_TARGET
{

    return disTex.Sample(sampleWrap, vsOut.texC);
}


