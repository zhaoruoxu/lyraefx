#if defined(_MSC_VER)
#pragma once
#endif

#ifndef LYRAE_CORE_RANDOM_H
#define LYRAE_CORE_RANDOM_H

#include "LyraeFX.h"

#define mtRandN     624


namespace LyraeFX
{
    class Random
    {
    public:
        Random() { Seed(0xF2710812); }
        Random(uint32_t seed)
        {
            if ( seed ) Seed(seed);
            else Seed(0xF2710812);
        }
        void Seed(uint32_t seed);
        float Rand();
        Vector RandUnitVector();
        uint32_t RandI();

    private:
        uint32_t mt[mtRandN];
        int mti;
    };

    extern Random gRandom;
}

#endif // LYRAE_CORE_RANDOM_H