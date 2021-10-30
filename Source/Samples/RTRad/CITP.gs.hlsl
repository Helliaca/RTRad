import Scene.Raster;


struct GSOut
{
    float2 texC : TEXCOORD;
    float4 posH : SV_POSITION;
    float3 posW : POSW;
    float3 normalW : NORMAL;
    float areaFactor : AREAFACTOR;
};

[maxvertexcount(3)]
void gmain(triangle VSOut input[3], inout TriangleStream<GSOut> OutputStream)
{
    GSOut output;

    // UV surface area
    float uvA = 0.5f * length(cross(float3(input[1].texC - input[0].texC, 0.f), float3(input[2].texC - input[1].texC, 0.f)));

    // real surface area
    float vvA = 0.5f * length(cross(input[1].posW - input[0].posW, input[2].posW - input[1].posW));

    for (uint i = 0; i < 3; i += 1)
    {
        output.posH = input[i].posH;
        output.texC = input[i].texC;
        output.posW = input[i].posW;
        output.normalW = input[i].normalW;

        output.areaFactor = uvA / vvA;

        OutputStream.Append(output);
    }

    OutputStream.RestartStrip();
}
