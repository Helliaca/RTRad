// Falcor / Slang imports to include shared code and data structures
//__import Shading;           // Imports ShaderCommon and DefaultVS, plus material evaluation
//__import DefaultVS;         // VertexOut declaration

import Scene.Raster;
import Scene.Shading;
import Utils.Sampling.TinyUniformSampleGenerator;
import Experimental.Scene.Lights.LightHelpers;
import Experimental.Scene.Material.StandardMaterial;
import RTRad.Voxel;


cbuffer PerFrameCB_ps {
    bool showTexRes;
    bool showVoxelMap;
    float4 interp_min;
    float4 interp_max;
    int mipmapLevel;

    float3 minPos;
    float3 maxPos;
};

Texture2D<float4> outputTex;

Texture3D<float4> voxTex;

SamplerState sampleWrap : register(s0);

bool xor(bool a, bool b) {
    return (a && !b) || (!a && b);
}

//float3 rayMarch(float3 start, float3 end) {
//    float voxRes;
//    voxTex.GetDimensions(voxRes, voxRes, voxRes);
//
//    for (float i = 0; i < 2.25f; i+=1.0f / 256.0f) {
//        float3 pos = start + i * (end - start);
//
//        if (max3(abs(pos)) > 1.0f) continue;
//
//        uint3 voxPos = worldSpaceToVoxelSpace(pos, minPos, maxPos, voxRes);
//
//        if (voxTex[voxPos].a > 0) {
//            return voxTex[voxPos].rgb;
//        }
//    }
//    return float3(0, 0, 0);
//}

float4 pmain(VSOut vsOut, uint triangleIndex : SV_PrimitiveID) : SV_TARGET
{

    float4 col = outputTex.SampleLevel(sampleWrap, vsOut.texC, mipmapLevel);

    col = col / (interp_max - interp_min);

    if (showTexRes) {
        float dim1;
        float dim2;
        outputTex.GetDimensions(dim1, dim2);

        vsOut.texC *= dim1 * 0.5f;
        vsOut.texC = fmod(vsOut.texC, 1.0);

        if(xor(vsOut.texC.x > 0.5f, vsOut.texC.y > 0.5f)) col = float4(1.0f);
        else col = float4(0.0f);
    }

    else if (showVoxelMap) {
        col.rgb = rayMarch(gScene.camera.getPosition(), vsOut.posW.xyz, minPos, maxPos, voxTex);
    }

    return col;
}


