#if defined(_MSC_VER)
#pragma once
#endif

#ifndef LYRAE_LIGHT_SPOTLIGHT_H
#define LYRAE_LIGHT_SPOTLIGHT_H

#include "LyraeFX.h"
#include "Geometry.h"
#include "Light.h"
#include "Random.h"

namespace LyraeFX
{
    class SpotLight : public Light
    {
    public:
        SpotLight(const Vector &pos, const Vector &dir, float a0, float a1, const Color &color, float intensity, float alpha = 0.f)
            :Light(color, intensity, alpha), mPosition(pos), mDirection(dir.GetNormalizedVector()), alpha0(a0), alpha1(a1) 
        {
            _ASSERT(alpha0 >= 0);
            _ASSERT(alpha1 >= alpha0);
            invDeltaAlpha = (alpha0 == alpha1 ? 0 : 1.f / (alpha1 - alpha0));
        }
        
        LightType GetType() { return SPOT_LIGHT; }
        Vector GetCentroid() { return mPosition; }

        Color Sample(const Vector &pos);
        Vector RandomDirection(Random &r)
        {
            Vector v;
            while ( true ) {
                v = r.RandUnitVector();
                float alpha = Degrees(acosf(v.Dot(mDirection)));
                if ( alpha < 0 ) alpha += 360.f;
                if ( alpha < alpha1 ) 
                    break;
            }
            return v;
        }

        Vector mPosition;
        Vector mDirection;
        float alpha0;
        float alpha1;
        float invDeltaAlpha;
    };
}

#endif // LYRAE_LIGHT_SPOTLIGHT_H