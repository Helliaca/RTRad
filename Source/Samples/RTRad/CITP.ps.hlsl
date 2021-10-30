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
    float4 nrm    : SV_Target1;
    float4 li0    : SV_Target2;
    float4 li1    : SV_Target3;
};


GBuffer pmain(VSOut vsOut, uint triangleIndex : SV_PrimitiveID)
{
    // coord transformation
    vsOut.posW = 0.5f * (vsOut.posW + float3(1.f));
    vsOut.normalW = 0.5f * (vsOut.normalW + float3(1.f));

    GBuffer o;
    o.pos = float4(vsOut.posW, 1.f);
    o.nrm = float4(vsOut.normalW, 1.f);

    // apply lighting
    if (vsOut.posW.y > 0.99f) {
        o.li0 = float4(1.f);
        o.li1 = float4(1.f);
    }
    else {
        o.li0 = float4(0.f);
        o.li1 = float4(0.f);
    }
    return o;
}


