import Scene.Raster;

cbuffer PerFrameCB {
    bool applyToModel;
};

VSOut vmain(VSIn vIn)
{
    VSOut vOut = defaultVS(vIn);

    if (!applyToModel) {
        float2 clipspace = vIn.texC * 2.0f - 1.0f; // Go from [0,1] to [-1,1]
        vOut.posH = float4(clipspace.x, -clipspace.y, 0.0f, 1.0f);
    }

    return vOut;
}
