#if defined(_MSC_VER)
#pragma once
#endif

#ifndef LYRAE_LIGHT_POINTLIGHT_H
#define LYRAE_LIGHT_POINTLIGHT_H

#include "../Core/LyraeFX.h"
#include "../Core/Light.h"

namespace LyraeFX
{
    class PointLight : public Light
    {
    public:
        PointLight(const Vector &pos)
            :mPosition(pos) {}

        PointLight(const Vector &pos, const Color &c, float intensity, float alpha = 0.f)
            :Light(c, intensity, alpha), mPosition(pos) {}

        LightType GetType() { return POINT_LIGHT; }
        Vector GetCentroid() { return mPosition; }
        inline Color Sample(const Vector &pos);

        Vector mPosition;
    };

    inline Color PointLight::Sample(const Vector &pos)
    {
        float v = minvAlphaSquared == 0.f ? 1.f :
            1.f / (1.f + (pos - mPosition).LengthSquared() * minvAlphaSquared);
        return mColor * mIntensity * v;
    }
}

#endif // LYRAE_LIGHT_POINTLIGHT_H