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
    float arf    : SV_Target2;
    float mat    : SV_Target3;
    float4 li0    : SV_Target4;
    float4 li1    : SV_Target5;
};


struct GSOut
{
    float2 texC : TEXCOORD;
    float4 posH : SV_POSITION;
    float3 posW : POSW;
    float3 normalW : NORMAL;
    float areaFactor : AREAFACTOR;
    uint materialID  : MATERIAL_ID;
};

GBuffer pmain(GSOut vsOut)
{
    // coord transformation
    vsOut.posW = 0.5f * (vsOut.posW + float3(1.f));
    vsOut.normalW = 0.5f * (vsOut.normalW + float3(1.f));

    GBuffer o;
    o.pos = float4(vsOut.posW, 1.f);
    o.nrm = float4(vsOut.normalW, 1.f);
    o.arf = vsOut.areaFactor;
    o.mat = vsOut.materialID;

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


