#include "TCylinder.h"
#include "../Config/EngineConfig.h"

namespace LyraeFX
{
    IntersectResult TCylinder::Intersect(const Ray &ray, float &adist)
    {
        _ASSERT( adist >= 0.f );
        Vector O = ray.mOrigin - mCenter;
        Vector D = ray.mDirection;

        float A = D.x * D.x + D.y * D.y;
        float B = 2.f * (O.x * D.x + O.y * D.y);
        float C = O.x * O.x + O.y * O.y - mSquaredRadius;
        float t0, t1;
        IntersectResult sResult = IR_MISS;

        float delta = B * B - 4 * A * C;
        if ( A == 0 || delta < 0 )
        {
            return IR_MISS;
        }
        float qdelta = sqrtf(delta);
        t0 = (-B + qdelta) / (2.f * A);
        t1 = (-B - qdelta) / (2.f * A);
        if ( t0 > t1 ) swap(t0, t1);
        float e = EPSILON;

        if ( t0 < 0 ) {
            if ( t1 < 0 ) return IR_MISS;
            if ( t1 < adist ) {
                float z = O.z + t1 * D.z;
                if ( abs(z) <= mHalfHeight ) {
                    adist = t1 - e;
                    return IR_HIT;
                }
            }
        } else {
            if ( t0 < adist ) {
                float z = O.z + t0 * D.z;
                if ( abs(z) < mHalfHeight ) {
                    adist = t0 - e;
                    return IR_HIT;
                } else if ( t1 < adist ) {
                    float z1 = O.z + t1 * D.z;
                    if ( abs(z1) < mHalfHeight ) {
                        adist = t1 - e;
                        return IR_HIT;
                    }
                }
            } 
        }

        return IR_MISS;
    }

    Color TCylinder::GetColor(const Vector &pos)
    {
        return mMaterial->mColor;
    }
}