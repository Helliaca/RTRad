uint InvCantorIndex(uint x, uint y, int texRes)
{
    if (x > y) {
        uint tmp = x;
        x = y;
        y = tmp;
    }

    x = texRes - x;
    return (x + y) * (x + y + 1) / 2 + x;
}

uint getBufferPos(uint2 self_c, uint2 other_c, int texRes)
{
    uint self_p = self_c.x + self_c.y * texRes;
    uint other_p = other_c.x + other_c.y * texRes;

    return InvCantorIndex(self_p, other_p, texRes);
}

bool getVisible(uint bufferpos, RWBuffer<uint> vis) {
    uint bitpos = bufferpos % 32;

    bufferpos = bufferpos / 32;

    uint v = vis[bufferpos];

    return ((v >> bitpos) & 1) > 0;
}

void setVisible(uint bufferpos, RWBuffer<uint> vis) {
    uint bitpos = bufferpos % 32;

    bufferpos = bufferpos / 32;

    uint v = vis[bufferpos];

    vis[bufferpos] = v | (1 << bitpos);
}
