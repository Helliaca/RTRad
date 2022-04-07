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
};

RWTexture3D<float4> voxTex;

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
    float3 posW = vsOut.posW.xyz;
    posW = 0.5f * (posW + float3(1, 1, 1)); // [0,1]
    posW = posW * 63.999f; // [0, 64]
    uint3 samp = (uint3)posW;
    voxTex[samp] = float4(vsOut.posW, 1);

    GBuffer o;
    o.pos = float4(vsOut.posW, 1.f);
    return o;
}


