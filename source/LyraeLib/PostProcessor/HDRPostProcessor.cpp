#include "HDRPostProcessor.h"
#include "BlurPostProcessor.h"
#include "ScaleBlur.h"
#include "../Core/LyraeFX.h"
#include "../Core/RenderTarget.h"
#include "../Core/Geometry.h"
#include "ToneMapping.h"

namespace LyraeFX
{
    void HDRPostProcessor::Process()
    {
        HDRRenderTarget ext(mWidth, mHeight);

        ToneMapping tone(mRT, 0.18f);
        tone.Process();

        ext.CopyFrom(*mRT);
        mBloomThreshold = 0.4f;


        for ( int y = 0; y < mHeight; y++ ) for ( int x = 0; x < mWidth; x++ )
        {
            Color *p = ext.mBuffer + (x + y * mWidth);
            float lum = 0.27f * p->x + 0.67f * p->y + 0.06f * p->z;
            lum = (lum - mBloomThreshold) / ( 1.f - mBloomThreshold);
            if ( lum < 0.f ) lum = 0.f;
            else if ( lum > 1.f ) lum = 1.f;
            *p *= lum;
        }

        HDRRenderTarget ext2(mWidth, mHeight);
        HDRRenderTarget ext4(mWidth, mHeight);
        ext2.CopyFrom(ext);
        ext4.CopyFrom(ext);
        ScaleBlur sblur2(&ext2, 2, 32);
        ScaleBlur sblur4(&ext4, 4, 64);
        sblur2.Process();
        sblur4.Process();
        BlurPostProcessor blur2(&ext2);
        BlurPostProcessor blur4(&ext4);
        blur2.Process();
        blur4.Process();
        blur4.Process();


        mRT->Merge(ext2, 1.0f, 0.2f);
        mRT->Merge(ext4, 1.0f, 0.4f);
        return;
        
    }
}