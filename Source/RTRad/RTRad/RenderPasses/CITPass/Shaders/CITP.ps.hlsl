// Falcor / Slang imports to include shared code and data structures
//__import Shading;           // Imports ShaderCommon and DefaultVS, plus material evaluation
//__import DefaultVS;         // VertexOut declaration

import Scene.Raster;
import Scene.Shading;
import Utils.Sampling.TinyUniformSampleGenerator;
import Experimental.Scene.Lights.LightHelpers;
import Experimental.Scene.Material.StandardMaterial;

struct GBuffer
{
    float4 pos    : SV_Target0;
    float4 nrm    : SV_Target1;
    float arf    : SV_Target2;
    float4 mat    : SV_Target3;
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

cbuffer PerFrameCB {
    float3 posOffset;
};

GBuffer pmain(GSOut vsOut)
{
    // coord transformation
    vsOut.posW = vsOut.posW - posOffset;
    vsOut.normalW = normalize(vsOut.normalW);// 0.5f * (vsOut.normalW + float3(1.f));

    GBuffer o;
    o.pos = float4(vsOut.posW, 1.f);
    o.nrm = float4(vsOut.normalW, 1.f);
    o.arf = vsOut.areaFactor;

    // Set mat
    o.mat = gScene.materials[vsOut.materialID].baseColor;
    MaterialResources mr = gScene.materialResources[vsOut.materialID];
    float4 mrc = mr.baseColor.Sample(mr.samplerState, vsOut.texC);
    if (mrc.a > 0.0f) {
        o.mat *= mrc;
    }
    o.mat.a = vsOut.materialID;

    o.li0 = float4(gScene.materials[vsOut.materialID].emissive, 1.f);
    o.li1 = float4(gScene.materials[vsOut.materialID].emissive, 1.f);


    return o;
}


