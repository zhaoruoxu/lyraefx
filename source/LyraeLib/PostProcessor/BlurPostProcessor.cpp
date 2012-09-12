#include "BlurPostProcessor.h"
#include "../Core/Geometry.h"
#include "../Core/LyraeFX.h"

namespace LyraeFX
{
    const int32_t BlurPostProcessor::DX[] = 
    {
        -1, 0, 1, 
        -1, 0, 1, 
        -1, 0, 1
    };
    const int32_t BlurPostProcessor::DY[] = 
    {
        -1, -1, -1, 
        0, 0, 0, 
        1, 1, 1
    };
    const float BlurPostProcessor::WEIGHT[] = 
    {
        1 / 16.f, 2 / 16.f, 1 / 16.f,
        2 / 16.f, 4 / 16.f, 2 / 16.f,
        1 / 16.f, 2 / 16.f, 1 / 16.f,
    };

    void BlurPostProcessor::Process()
    {
        Color *prt = mRT->mBuffer;
        Color *ptemp = new Color[mWidth * mHeight];
        for ( int32_t h = 0; h < mHeight; h++ ) for ( int32_t w = 0; w < mWidth; w++ ) 
        {
            int32_t pos = w + h * mWidth;
            ptemp[pos] = Color(0.f, 0.f, 0.f);
            for ( int32_t i = 0; i < 9; i++ )
            {
                int w1 = w + DX[i];
                int h1 = h + DY[i];
                if ( w1 < 0 || w1 >= mWidth || h1 < 0 || h1 >= mHeight ) continue;
                _ASSERT(w1 >= 0 && w1 < mWidth);
                _ASSERT(h1 >= 0 && h1 < mHeight);
                ptemp[pos] += prt[w1 + h1 * mWidth] * WEIGHT[i];
            }
        }
        for ( int32_t pos = 0; pos < mWidth * mHeight; pos++ ) 
        {
            prt[pos] = ptemp[pos];
        }
        delete [] ptemp;
    }
}