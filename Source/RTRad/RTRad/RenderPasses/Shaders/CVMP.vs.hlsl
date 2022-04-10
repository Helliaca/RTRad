import Scene.Raster;

VSOut main(VSIn vIn)
{
    VSOut vOut;
    const GeometryInstanceID instanceID = { vIn.meshInstanceID };

    //float4x4 worldMat = gScene.getWorldMatrix(instanceID);

    //float3 posW = mul(float4(vIn.pos, 1.f), worldMat).xyz;
    float3 posW = vIn.pos;

    vOut.posW = posW;

    vOut.posH = float4(posW, 1.f);// mul(float4(posW, 1.f), gScene.camera.data.viewMat);

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
    vOut.prevPosH = mul(float4(prevPosW, 1.f), gScene.camera.data.prevViewProjMatNoJitter);

    return vOut;

}
