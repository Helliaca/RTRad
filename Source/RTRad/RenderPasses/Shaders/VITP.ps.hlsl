// Falcor / Slang imports to include shared code and data structures
//__import Shading;           // Imports ShaderCommon and DefaultVS, plus material evaluation
//__import DefaultVS;         // VertexOut declaration

import Scene.Raster;
import Scene.Shading;
import Utils.Sampling.TinyUniformSampleGenerator;
import Experimental.Scene.Lights.LightHelpers;
import Experimental.Scene.Material.StandardMaterial;

cbuffer PerFrameCB2 {
    bool showTexRes;
    float4 interp_min;
    float4 interp_max;
    int mipmapLevel;
};

Texture2D<float4> disTex;

Texture3D<float4> voxTex;

SamplerState sampleWrap : register(s0);

bool xor(bool a, bool b) {
    return (a && !b) || (!a && b);
}

uint3 toVoxelSpace(float3 posW) {
    posW = 0.5f * (posW + float3(1, 1, 1)); // [0,1]
    posW = posW * 63.9999f; // [0, 64]
    return (uint3)posW;
}

float max3(float3 v) {
    return max(v.x, max(v.y, v.z));
}

float3 rayMarch(float3 start, float3 end) {
    for (float i = 0; i < 2.25f; i+=1.0f / 256.0f) {
        float3 pos = start + i * (end - start);

        if (max3(abs(pos)) > 1.0f) continue;

        if (voxTex[toVoxelSpace(pos)].a > 0) {
            return voxTex[toVoxelSpace(pos)].rgb;
        }
    }
    return float3(0, 0, 0);
}

float4 pmain(VSOut vsOut, uint triangleIndex : SV_PrimitiveID) : SV_TARGET
{

    float4 col = disTex.SampleLevel(sampleWrap, vsOut.texC, mipmapLevel);

    col = col / (interp_max - interp_min);

    //Temporary ammendment: We show voxelmap data when showTexRes.
    // get rid of this and re-enable the if below to go back to how it was before
    if (showTexRes) {
        col.rgb = rayMarch(gScene.camera.getPosition(), vsOut.posW.xyz);

        //float3 posW = vsOut.posW.xyz;
        //posW = 0.5f*(posW + float3(1, 1, 1)); // [0,1]
        //posW = posW * 63.9999f; // [0, 64]
        //uint3 samp = (uint3)posW;
        //col = voxTex[samp];

        //Camera cam = gScene.camera;
        //col.xyz = cam.getPosition();
    }

    /*if (showTexRes) {
        float dim1;
        float dim2;
        disTex.GetDimensions(dim1, dim2);

        vsOut.texC *= dim1 * 0.5f;
        vsOut.texC = fmod(vsOut.texC, 1.0);

        if(xor(vsOut.texC.x > 0.5f, vsOut.texC.y > 0.5f)) col = float4(1.0f);
        else col = float4(0.0f);
    }*/

    return col;
}


