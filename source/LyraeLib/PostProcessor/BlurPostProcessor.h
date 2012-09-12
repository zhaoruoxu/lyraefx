#if defined(_MSC_VER)
#pragma once
#endif

#ifndef LYRAE_POSTPROCESSOR_BLURPOSTPROCESSOR_H
#define LYRAE_POSTPROCESSOR_BLURPOSTPROCESSOR_H

#include "../Core/LyraeFX.h"
#include "../Core/PostProcessor.h"
#include "../Core/RenderTarget.h"

namespace LyraeFX
{
    class BlurPostProcessor : public PostProcessor
    {
    public:
        BlurPostProcessor() : mRT(NULL), mWidth(0), mHeight(0) {}
        BlurPostProcessor(HDRRenderTarget *p)
            :mRT(p)
        {
            if ( p ) {
                mWidth = p->mWidth;
                mHeight = p->mHeight;
            }
        }
        virtual ~BlurPostProcessor() {}

        void Process();

        HDRRenderTarget *mRT;
        int32_t mWidth, mHeight;

        static const int32_t DX[9];
        static const int32_t DY[9];
        static const float WEIGHT[9];

    };
}

#endif // LYRAE_POSTPROCESSOR_BLURPOSTPROCESSOR_H