#if defined(_MSC_VER)
#pragma once
#endif

#ifndef LYRAE_CORE_LYRAEAPI_H
#define LYRAE_CORE_LYRAEAPI_H

#include "LyraeFX.h"
#include "Geometry.h"

using namespace LyraeFX;

namespace LyraeFX
{
    void HDRToRGB(Pixel *dest, Color *src, int32_t width, int32_t height);
    bool LyraeFXRender();
}



#endif // LYRAE_CORE_LYRAEAPI_H