// Falcor / Slang imports to include shared code and data structures
//__import Shading;           // Imports ShaderCommon and DefaultVS, plus material evaluation
//__import DefaultVS;         // VertexOut declaration

import Scene.Raster;
import Scene.Shading;
import Utils.Sampling.TinyUniformSampleGenerator;
import Experimental.Scene.Lights.LightHelpers;
import Experimental.Scene.Material.StandardMaterial;
import RTRad.RTRad.Slang.Voxel;

struct GBuffer
{

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
    float3 minPos;
    float3 maxPos;
};

GBuffer pmain(GSOut vsOut)
{
    float voxRes;
    voxTex.GetDimensions(voxRes, voxRes, voxRes);

    uint3 samp = worldSpaceToVoxelSpace(vsOut.posW.xyz, minPos, maxPos, voxRes);
    voxTex[samp] = float4((vsOut.posW.xyz-minPos)/(maxPos-minPos), 1);

    GBuffer o;
    return o;
}


