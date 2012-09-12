#if defined(_MSC_VER)
#pragma once
#endif

#ifndef LYRAE_CORE_ADAPTIVESAMPLER_H
#define LYRAE_CORE_ADAPTIVESAMPLER_H

#include "LyraeFX.h"
#include "Geometry.h"

namespace LyraeFX 
{
    void RequestSamples(float *dest, int32_t dim);
    bool NeedSupersampling(Color *c, int32_t n, float threshold);
}


#endif // LYRAE_CORE_ADAPTIVESAMPLER_H