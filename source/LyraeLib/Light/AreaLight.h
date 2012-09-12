#if defined(_MSC_VER)
#pragma once
#endif

#ifndef LYRAE_LIGHT_AREALIGHT_H
#define LYRAE_LIGHT_AREALIGHT_H

#include "../Core/LyraeFX.h"

namespace LyraeFX
{
    class AreaLight : public Light
    {
    public:
        AreaLight(const Vector &pos, const Vector &size, const Color &c, float intensity, float alpha = 0.f)
            : Light(c, intensity, alpha), mPosition(pos), mSize(size) {}

        LightType GetType() { return AREA_LIGHT; }
        Vector GetCentroid() { return mPosition + mSize * 0.5f; }

        inline Color Sample(const Vector &pos);

        Vector mPosition;
        Vector mSize;
    }; // class AreaLight

    inline Color AreaLight::Sample(const Vector &pos)
    {
        float v = minvAlphaSquared == 0.f ? 1.f :
            1.f / ( 1.f + (pos - mPosition).LengthSquared() * minvAlphaSquared);
        return mColor * mIntensity * v; 
    }

} // namespace LyraeFX


#endif // LYRAE_LIGHT_AREALIGHT_H