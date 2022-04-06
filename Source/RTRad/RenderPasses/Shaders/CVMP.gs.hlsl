import Scene.Raster;


struct GSOut
{
    float2 texC : TEXCOORD;
    float4 posH : SV_POSITION;
    float3 posW : POSW;
    float3 normalW : NORMAL;
    float areaFactor : AREAFACTOR;
    uint materialID  : MATERIAL_ID;
};

[maxvertexcount(3)]
void gmain(triangle VSOut input[3], inout TriangleStream<GSOut> OutputStream)
{
    GSOut output;

    // normal
    float3 faceNormal = abs(cross(input[1].posW - input[0].posW, input[2].posW - input[0].posW));

    // dominatn axis
    float dominantAxis = max(faceNormal.x, max(faceNormal.y, faceNormal.z));

    for (uint i = 0; i < 3; i += 1)
    {
        output.posW = input[i].posW;
        output.normalW = input[i].normalW;

        if (dominantAxis == faceNormal.x) output.posH = float4(output.posW.zyx, 1);
        else if (dominantAxis == faceNormal.y) output.posH = float4(output.posW.xzy, 1);
        else if (dominantAxis == faceNormal.z) output.posH = float4(output.posW.xyz, 1);

        OutputStream.Append(output);
    }

    OutputStream.RestartStrip();
}
