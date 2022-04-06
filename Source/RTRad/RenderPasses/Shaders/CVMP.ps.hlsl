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
    // coord transformation
    vsOut.posW = vsOut.posW - posOffset;
    vsOut.normalW = 0.5f * (vsOut.normalW + float3(1.f));

    GBuffer o;

    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 64; j++) {
            for (int u = 0; u < 64; u++) {
                uint3 coord = uint3(i, j, u);
                voxTex[coord] = float4(0, 1, 0, 1);
            }
        }
    }

    //uint3 coord = uint3(0, 0, 0);
    //voxTex[coord] = float4(0, 1, 0, 1);

    //o.pos = float4(1, 0, 1, 1.f);
    //o.pos = voxTex[coord];


    return o;
}


