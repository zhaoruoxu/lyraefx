#if defined(_MSC_VER)
#pragma once
#endif

#ifndef LYRAE_CORE_POSTPROCESSOR_H
#define LYRAE_CORE_POSTPROCESSOR_H


#include "LyraeFX.h"
#include "RenderTarget.h"

namespace LyraeFX
{
    class PostProcessor
    {
    public:
        PostProcessor() {}
        virtual ~PostProcessor() {}

        virtual void Process() = 0;

    }; // class PostProcessor

} // namespace LyraeFX


#endif // LYRAE_CORE_POSTPROCESSOR_H