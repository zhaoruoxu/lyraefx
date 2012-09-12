#include "SpotLight.h"

namespace LyraeFX
{

    Color SpotLight::Sample(const Vector &pos)
    {
        Vector pdir = pos - mPosition;
        pdir.Normalize();
        float alpha = acosf(mDirection.Dot(pdir)) * INV_PI * 180.f;
        if ( alpha < 0 ) alpha += 360.f;
        _ASSERT(alpha >= 0);
        float v = minvAlphaSquared == 0.f ? 1.f :
            1.f / (1.f + (pos - mPosition).LengthSquared() * minvAlphaSquared);
        if ( alpha < alpha0 ) return mColor * mIntensity * v;
        else if ( alpha < alpha1 ) return mColor * mIntensity * ((alpha1 - alpha) * (invDeltaAlpha * v));
        else return Color(0, 0, 0);
    }
}