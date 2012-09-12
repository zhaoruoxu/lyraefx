#include "ScaleBlur.h"
#include "../Core/PostProcessor.h"
#include "../Core/LyraeFX.h"
#include "../Core/RenderTarget.h"
#include "BlurPostProcessor.h"

namespace LyraeFX
{
    void ScaleBlur::Process()
    {
        int32_t dwidth = mWidth / mDivision + 1;
        int32_t dheight = mHeight / mDivision + 1;
        if ( 0 == dwidth || 0 == dheight ) return;

        HDRRenderTarget rt(dwidth, dheight);
        float scale = (float) (mWidth * mHeight) / (float) (dwidth * dheight);
        float rsclae = 1.f / scale;
        Color *temp = rt.mBuffer;
        for ( int32_t y = 0; y < mHeight; y++ ) for ( int32_t x = 0; x < mWidth; x++ )
        {
            int32_t xx = x / mDivision;
            int32_t yy = y / mDivision;
            temp[xx + yy * dwidth] += mRT->mBuffer[x + y * mWidth] * rsclae;
        }

        BlurPostProcessor blur(&rt);
        for ( int32_t i = 0; i < mBlur; i++ )
        {
            blur.Process();
        }

        for ( int32_t y = 0; y < mHeight; y++ ) for ( int32_t x = 0; x < mWidth; x++ )
        {
            int32_t xx = x / mDivision;
            int32_t yy = y / mDivision;
            mRT->mBuffer[x + y * mWidth] = temp[xx + yy * dwidth];
        }
    }
}