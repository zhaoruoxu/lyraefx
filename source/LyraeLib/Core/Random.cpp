#include "Random.h"
#include "LyraeFX.h"
#include "Geometry.h"

namespace LyraeFX
{
    Random gRandom;

    unsigned short mtRand_xsubi[3] = { 723, 32761, 44444 };

#define M 397
#define MATRIX_A 0x9908b0df
#define UPPER_MASK 0x80000000
#define LOWER_MASK 0x7fffffff

#define TEMPERING_MASK_B 0x9d2c5680
#define TEMPERING_MASK_C 0xefc60000
#define TEMPERING_SHIFT_U(y)  (y >> 11)
#define TEMPERING_SHIFT_S(y)  (y << 7)
#define TEMPERING_SHIFT_T(y)  (y << 15)
#define TEMPERING_SHIFT_L(y)  (y >> 18)

    void Random::Seed( uint32_t seed )
    {
        mt[0]= seed & 0xffffffff;
        for ( mti = 1; mti < mtRandN; mti++ ) mt[mti] = (69069 * mt[mti - 1]) & 0xffffffff;
        unsigned long s = 373737;
        for ( mti = 1; mti < mtRandN; mti++ )
        {
            mt[mti] ^= s;
            s = s * 5531 + 81547;
            s ^= (s >> 9) ^ (s << 19);
        }
    }

    Vector Random::RandUnitVector()
    {
        Vector r(Rand() - 0.5f, Rand() - 0.5f, Rand() - 0.5f);
        return r.GetNormalizedVector();
    }

    float Random::Rand()
    {
        uint32_t y;
        static uint32_t mag01[2] = {0x0, MATRIX_A};
        if (mti >= mtRandN) 
        {
            int kk;
            for (kk=0;kk<mtRandN-M;kk++) 
            {
                y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
                mt[kk] = mt[kk+M] ^ (y >> 1) ^ mag01[y & 0x1];
            }
            for (;kk<mtRandN-1;kk++) 
            {
                y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
                mt[kk] = mt[kk+(M-mtRandN)] ^ (y >> 1) ^ mag01[y & 0x1];
            }
            y = (mt[mtRandN-1]&UPPER_MASK)|(mt[0]&LOWER_MASK);
            mt[mtRandN-1] = mt[M-1] ^ (y >> 1) ^ mag01[y & 0x1];
            mti = 0;
        }
        y = mt[mti++];
        y ^= TEMPERING_SHIFT_U(y);
        y ^= TEMPERING_SHIFT_S(y) & TEMPERING_MASK_B;
        y ^= TEMPERING_SHIFT_T(y) & TEMPERING_MASK_C;
        y ^= TEMPERING_SHIFT_L(y);
        return ( (float)y * 2.3283064370807974e-10f );
    }

    uint32_t Random::RandI()
    {
        uint32_t y;
        static uint32_t mag01[2] = { 0x0, MATRIX_A };
        if (mti >= mtRandN) 
        {
            int kk;
            for (kk=0;kk<mtRandN-M;kk++) 
            {
                y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
                mt[kk] = mt[kk+M] ^ (y >> 1) ^ mag01[y & 0x1];
            }
            for (;kk<mtRandN-1;kk++) 
            {
                y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
                mt[kk] = mt[kk+(M-mtRandN)] ^ (y >> 1) ^ mag01[y & 0x1];
            }
            y = (mt[mtRandN-1]&UPPER_MASK)|(mt[0]&LOWER_MASK);
            mt[mtRandN-1] = mt[M-1] ^ (y >> 1) ^ mag01[y & 0x1];
            mti = 0;
        }
        y = mt[mti++];
        y ^= TEMPERING_SHIFT_U(y);
        y ^= TEMPERING_SHIFT_S(y) & TEMPERING_MASK_B;
        y ^= TEMPERING_SHIFT_T(y) & TEMPERING_MASK_C;
        y ^= TEMPERING_SHIFT_L(y);
        return y;
    }
}