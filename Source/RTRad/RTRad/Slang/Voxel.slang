float max3(float3 v) {
    return max(v.x, max(v.y, v.z));
}

float min3(float3 v) {
    return min(v.x, min(v.y, v.z));
}

uint3 worldSpaceToVoxelSpace(float3 pos, float3 minpos, float3 maxpos, float voxRes) {
    pos = (pos - minpos) / (maxpos - minpos); // [0, 1]

    pos *= (voxRes - 0.51f); // [0, voxRes]

    return (uint3)pos;
}

// Ray march for visibility. This function can be used instead of TraceRay to determine visibility.
bool vRayMarch(float3 start, float3 end, Texture3D<float4> voxTex, float3 minPos, float3 maxPos) {
    float voxRes;
    voxTex.GetDimensions(voxRes, voxRes, voxRes);

    //epsilon
    start += normalize(end - start) * 0.07f;
    end += normalize(start - end) * 0.07f;

    float step = 1.0f / voxRes;

    for (float i = 0; i < 1.0f; i += step) {
        float3 pos = start + i * (end - start);

        uint3 voxPos = worldSpaceToVoxelSpace(pos, minPos, maxPos, voxRes);

        if (voxTex[voxPos].a > 0) {
            return false;
        }
    }
    return true;
}

// This function is used for VISUALIZING the voxel map. Eg, rays are marched from the camera
float3 vRayMarchCol(float3 start, float3 end, Texture3D<float4> voxTex, float3 minPos, float3 maxPos) {
    float voxRes;
    voxTex.GetDimensions(voxRes, voxRes, voxRes);

    for (float i = 0; i < 3.0f; i += 1.0f / 256.0f) {
        float3 pos = start + i * (end - start);

        uint3 voxPos = worldSpaceToVoxelSpace(pos, minPos, maxPos, voxRes);
        if (max3(abs(voxPos)) > voxRes || min3(abs(voxPos)) < 0) continue;

        if (voxTex[voxPos].a > 0) {
            return voxTex[voxPos].rgb;
        }
    }
    return float3(0,0,0);
}
