// Falcor / Slang imports to include shared code and data structures
//__import Shading;           // Imports ShaderCommon and DefaultVS, plus material evaluation
//__import DefaultVS;         // VertexOut declaration

import Scene.Raster;
import Scene.Shading;
import Utils.Sampling.TinyUniformSampleGenerator;
import Experimental.Scene.Lights.LightHelpers;
import Experimental.Scene.Material.StandardMaterial;

cbuffer PerFrameCB2 {
    bool treatAsMatIDs;
    bool showTexRes;
};

Texture2D<float4> disTex;

SamplerState sampleWrap : register(s0);

bool xor(bool a, bool b) {
    return (a && !b) || (!a && b);
}

float4 pmain(VSOut vsOut, uint triangleIndex : SV_PrimitiveID) : SV_TARGET
{

    float4 col = disTex.Sample(sampleWrap, vsOut.texC);

    if (treatAsMatIDs) {
        uint matID = (uint) col.r;
        col = gScene.materials[matID].baseColor;
    }

    if (showTexRes) {
        float dim1;
        float dim2;
        disTex.GetDimensions(dim1, dim2);

        vsOut.texC *= dim1 * 0.5f;
        vsOut.texC = fmod(vsOut.texC, 1.0);

        if(xor(vsOut.texC.x > 0.5f, vsOut.texC.y > 0.5f)) col = float4(1.0f);
        else col = float4(0.0f);
    }

    return col;
}


