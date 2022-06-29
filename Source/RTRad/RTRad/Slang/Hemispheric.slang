

float3 simplePerp(float3 v, float3 q)
{
    v = normalize(v);
    q = normalize(q);
    return cross(v, q);
}
