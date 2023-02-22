#pragma once
#include <stdint.h>

static float Float16To32(uint16_t in)
{
    uint32_t t1;
    uint32_t t2;
    uint32_t t3;

    t1 = in & 0x7fff;
    t2 = in & 0x8000;
    t3 = in & 0x7c00;

    t1 <<= 13;
    t2 <<= 16;
    t1 += 0x38000000;            
    t1 |= t2;

    return *((float*)&t1);
}
