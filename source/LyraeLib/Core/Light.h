#if defined(_MSC_VER)
#pragma once
#endif

#ifndef LYRAE_CORE_LIGHT_H
#define LYRAE_CORE_LIGHT_H

#include "LyraeFX.h"
#include "Random.h"

namespace LyraeFX
{
    typedef enum _LightType
    {
        POINT_LIGHT,
        AREA_LIGHT,
        VIRTUAL_LIGHT,
        SPOT_LIGHT
    } LightType;

    class Light
    {
        friend class SimpleLight;
    public:
        Light() : mColor(1.f, 1.f, 1.f), mIntensity(1.f) {}
        Light(const Color &c, float intensity, float a = 0.f)
            :mColor(c), mIntensity(intensity), mAlpha(a) 
        {
            minvAlphaSquared = mAlpha == 0 ? 0 : 1.f / (mAlpha * mAlpha);
        }

        virtual LightType GetType() = 0;
        virtual Vector GetCentroid() = 0;
        virtual Color Sample(const Vector &pos)
        {
            return mColor * mIntensity;
        }
        virtual Vector RandomDirection(Random &r)
        {
            return r.RandUnitVector();
        }
        float mIntensity;

    protected:
        Color mColor;
        float mAlpha;
        float minvAlphaSquared;
        
    }; // class Light

} // namespace LyraeFX

#endif // LYRAE_CORE_LIGHT_H