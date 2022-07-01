

float3x3 getTangentToWorldMatrix(float3 surface_normal) {
    surface_normal = normalize(surface_normal);

    float3 tangent = cross(surface_normal, float3(0.0, 0.0, 1.0));
    float3 bitangent = cross(surface_normal, float3(0.0, 1.0, 0.0));

    if (length(bitangent) > length(tangent))
    {
        tangent = bitangent;
    }

    tangent = normalize(tangent);

    bitangent = cross(surface_normal, tangent);
    bitangent = normalize(bitangent);

    float3x3 m = {
        tangent,
        bitangent,
        surface_normal,
    };
    return transpose(m);
}
