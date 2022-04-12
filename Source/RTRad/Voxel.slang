
float3 rayMarch(float3 start, float3 end, float3 minPos, float3 maxPos, Texture3D<float4> voxTex) {
    float voxRes;
    voxTex.GetDimensions(voxRes, voxRes, voxRes);

    for (float i = 0; i < 2.25f; i+=1.0f / 256.0f) {
        float3 pos = start + i * (end - start);

        uint3 voxPos = worldSpaceToVoxelSpace(pos, minPos, maxPos, voxRes);
        if (max3(abs(voxPos)) > voxRes) continue;

        if (voxTex[voxPos].a > 0) {
            return voxTex[voxPos].rgb;
        }
    }
    return float3(0, 0, 0);
}

float max3(float3 v) {
    return max(v.x, max(v.y, v.z));
}

uint3 worldSpaceToVoxelSpace(float3 pos, float3 minpos, float3 maxpos, float voxRes) {
    pos = (pos - minpos) / (maxpos - minpos); // [0, 1]

    pos *= (voxRes); // [0, voxRes]

    return (uint3)pos;
}
