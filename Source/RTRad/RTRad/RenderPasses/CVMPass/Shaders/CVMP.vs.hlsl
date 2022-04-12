import Scene.Raster;

cbuffer PerFrameCB_vs {
    float3 posOffset;
    float3 minPos;
    float3 maxPos;
};

VSOut vmain(VSIn vIn)
{
    VSOut vOut;
    const GeometryInstanceID instanceID = { vIn.meshInstanceID };

    //float4x4 worldMat = gScene.getWorldMatrix(instanceID);

    //float3 posW = mul(float4(vIn.pos, 1.f), worldMat).xyz;
    float3 posW = vIn.pos;

    vOut.posW = posW;

    float3 posN = (posW.xyz - minPos) / (maxPos - minPos);  // [0, 1]
    posN = (2.0f * posN) - float3(1, 1, 1);                 // [-1, 1] (clippos)
    vOut.posH = float4(posN, 1);

    vOut.instanceID = instanceID;
    vOut.materialID = gScene.getMaterialID(instanceID);

    vOut.texC = vIn.texC;
    vOut.normalW = mul(vIn.unpack().normal, gScene.getInverseTransposeWorldMatrix(instanceID));
    float4 tangent = vIn.unpack().tangent;
    vOut.tangentW = float4(mul(tangent.xyz, (float3x3)gScene.getWorldMatrix(instanceID)), tangent.w);

    // Compute the vertex position in the previous frame.
    float3 prevPos = vIn.pos;
    MeshInstanceData meshInstance = gScene.getMeshInstance(instanceID);
    if (meshInstance.hasDynamicData())
    {
        uint dynamicVertexIndex = gScene.meshes[meshInstance.meshID].dynamicVbOffset + vIn.vertexID;
        prevPos = gScene.prevVertices[dynamicVertexIndex].position;
    }
    float3 prevPosW = mul(float4(prevPos, 1.f), gScene.getPrevWorldMatrix(instanceID)).xyz;
    vOut.prevPosH = vOut.posH;

    return vOut;

}
