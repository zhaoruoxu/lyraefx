#if defined(_MSC_VER)
#pragma once
#endif

#ifndef LYRAE_POSTPROCESSOR_SCALEBLUR_H
#define LYRAE_POSTPROCESSOR_SCALEBLUR_H

#include "../Core/LyraeFX.h"
#include "../Core/PostProcessor.h"
#include "../Core/RenderTarget.h"

namespace LyraeFX
{
    class ScaleBlur : public PostProcessor
    {
    public:
        ScaleBlur(HDRRenderTarget *p, int32_t div, int32_t blur)
            :mRT(p), mDivision(div), mBlur(blur)
        {
            if ( p ) {
                mWidth = p->mWidth;
                mHeight = p->mHeight;
            }
        }
        void Process();

        int32_t mDivision;
        int32_t mBlur;
        HDRRenderTarget *mRT;
        int32_t mWidth, mHeight;
    };
}

#endif // LYRAE_POSTPROCESSOR_SCALEBLUR_H