#if defined(_MSC_VER)
#pragma once
#endif

#ifndef LYRAE_POSTPROCESSOR_TONEMAPPING_H
#define LYRAE_POSTPROCESSOR_TONEMAPPING_H

#include "../Core/LyraeFX.h"
#include "../Core/PostProcessor.h"
#include "../Core/Geometry.h"

namespace LyraeFX
{

    class ToneMapping : public PostProcessor
    {
    public:
        ToneMapping(HDRRenderTarget *p, float a)
            : mRT(p), mAlpha(a)
        {
            if ( p ) {
                mWidth = p->mWidth;
                mHeight = p->mHeight;
            }
        }

        void Process();
        float calculateAverageLuminace();
        float calculateLuminace(Color *c)
        {
            return 0.299f * c->x + 0.587f * c->y + 0.114f * c->z;
        }
        
        HDRRenderTarget *mRT;
        float mAlpha;
        int32_t mWidth, mHeight;

        inline void RGB2YUV(Color &rgb, Color &yuv);
        inline void YUV2RGB(Color &yuv, Color &rgb);
    };

    inline void ToneMapping::RGB2YUV(Color &rgb, Color &yuv)
    {
        yuv.x = 0.299f * rgb.x + 0.587f * rgb.y + 0.114f * rgb.z;
        yuv.y = -0.14713f * rgb.x - 0.28886f * rgb.y + 0.436f * rgb.z;
        yuv.z = 0.615f * rgb.x - 0.51499f * rgb.y - 0.10001f * rgb.z;
    }

    inline void ToneMapping::YUV2RGB(Color &yuv, Color &rgb)
    {
        rgb.x = yuv.x + 1.13983f * yuv.z;
        rgb.y = yuv.x - 0.39465f * yuv.y - 0.58060f * yuv.z;
        rgb.z = yuv.x + 2.03211f * yuv.y;
    }
}

#endif // LYRAE_POSTPROCESSOR_TONEMAPPING_H