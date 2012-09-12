#if defined(_MSC_VER)
#pragma once
#endif

#ifndef LYRAE_LIGHT_VIRTUALLIGHT_H
#define LYRAE_LIGHT_VIRTUALLIGHT_H

#include "LyraeFX.h"
#include "Light.h"
#include "Shape.h"

namespace LyraeFX
{
    class VirtualLight : public Light
    {
    public:
        VirtualLight(const Vector &pos, const Color &color, float intensity)
            :Light(color, intensity), mPosition(pos), mShapePtr(NULL)
        {
        }

        VirtualLight(const Vector &pos, const Color &color, float intensity, Shape *sptr)
            :Light(color, intensity), mPosition(pos), mShapePtr(sptr)
        {
        }

        LightType GetType() { return VIRTUAL_LIGHT; }
        Vector GetCentroid() { return mPosition; }

        Color Sample(const Vector &pos) { return mColor * mIntensity; }

        Vector mPosition;
        Shape *mShapePtr;
    };
}

#endif // LYRAE_LIGHT_VIRTUALLIGHT_H