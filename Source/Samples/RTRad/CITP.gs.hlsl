import Scene.Raster;


struct GSOut
{
    float2 texC : TEXCOORD;
    float4 posH : SV_POSITION;
    float3 posW : POSW;
    float3 normalW : NORMAL;
};


[maxvertexcount(3)]
void gmain(triangle VSOut input[3], inout TriangleStream<GSOut> OutputStream)
{
    GSOut output;

    for (uint i = 0; i < 3; i += 1)
    {
        output.posH = input[i].posH;
        output.texC = input[i].texC;
        output.posW = input[i].posW;
        output.normalW = input[i].normalW;

        OutputStream.Append(output);
    }

    OutputStream.RestartStrip();
}
