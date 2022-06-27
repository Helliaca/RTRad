// Hash function from H. Schechter & R. Bridson, goo.gl/RXiKaH
uint Hash(uint seed)
{
    seed = (seed ^ 61) ^ (seed >> 16);
    seed *= 9;
    seed = seed ^ (seed >> 4);
    seed *= 0x27d4eb2d;
    seed = seed ^ (seed >> 15);
    return seed;
}

uint random(uint seed, uint max)
{
    return Hash(seed) % max;
}

uint2 random(uint2 seed, uint2 max)
{
    return uint2(
        Hash(seed.x) % max.x,
        Hash(seed.y) % max.y
    );
}
