



uint3 worldSpaceToVoxelSpace(float3 pos, float3 minpos, float3 maxpos, float voxRes) {
    pos = (pos - minpos) / (maxpos - minpos); // [0, 1]

    pos *= (voxRes - 0.51f); // [0, voxRes]

    return (uint3)pos;
}
