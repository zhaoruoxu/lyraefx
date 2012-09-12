#if defined(_MSC_VER)
#pragma once
#endif

#ifndef LYRAE_POSTPROCESSOR_HDRPOSTPROCESSOR_H
#define LYRAE_POSTPROCESSOR_HDRPOSTPROCESSOR_H

#include "../Core/LyraeFX.h"
#include "../Core/PostProcessor.h"
#include "../Core/RenderTarget.h"

namespace LyraeFX
{
    class HDRPostProcessor : public PostProcessor
    {
    public:
        //HDRPostProcessor() : mRT(NULL), mWidth(0), mHeight(0) {}
        HDRPostProcessor(HDRRenderTarget *p)
            :mRT(p), mBloomThreshold(0.5f), mBlurCount(16), mAlpha(0.4f)
        {
            if ( p ) {
                mWidth = p->mWidth;
                mHeight = p->mHeight;
            }
        }
        virtual ~HDRPostProcessor() {}

        void Process();


        HDRRenderTarget *mRT;
        int32_t mWidth, mHeight;
        float mBloomThreshold;
        int32_t mBlurCount;
        float mAlpha;
    };
}

#endif // LYRAE_POSTPROCESSOR_HDRPOSTPROCESSOR_H