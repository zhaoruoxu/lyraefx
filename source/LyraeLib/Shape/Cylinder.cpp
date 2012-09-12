#include "Cylinder.h"
#include "../Config/EngineConfig.h"

namespace LyraeFX
{
    IntersectResult Cylinder::Intersect(const Ray &ray, float &adist)
    {
        assert( adist >= 0.f );
        Vector O = ray.mOrigin - mCenter;
        Vector D = ray.mDirection;

        const float maxDist = MAXDISTANCE;
        float t2 = maxDist;
        float t3 = maxDist;
        IntersectResult tResult = IR_MISS;
        float tf = maxDist;
        if ( D.z != 0 )
        {
            t2 = (mHalfHeight - O.z) / D.z;
            t3 = (-mHalfHeight - O.z) / D.z;
            if ( t2 > t3 ) swap(t2, t3);
            if ( t2 < 0 )
            {
                if ( t3 > 0 && t3 < adist )
                {
                    Vector T3 = O + D * t3;
                    T3.z = 0;
                    if ( T3.LengthSquared() < mSquaredRadius )
                    {
                        tf = t3;
                        tResult = IR_HITINSIDE;
                        goto side;
                    }
                }
            }
            else
            {
                if ( t2 < adist )
                {
                    Vector T2 = O + D * t2;
                    T2.z = 0;
                    if ( T2.LengthSquared() < mSquaredRadius )
                    {
                        tf = t2;
                        tResult = IR_HIT;
                        goto side;
                    }
                }
            }
        }

    side:
        float A = D.x * D.x + D.y * D.y;
        float B = 2.f * (O.x * D.x + O.y * D.y);
        float C = O.x * O.x + O.y * O.y - mSquaredRadius;
        float t0, t1;
        IntersectResult sResult = IR_MISS;
        float ts = maxDist;

        float delta = B * B - 4 * A * C;
        if ( A == 0 || delta < 0 )
        {
            return IR_MISS;
        }
        float qdelta = sqrtf(delta);
        t0 = (-B + qdelta) / (2.f * A);
        t1 = (-B - qdelta) / (2.f * A);
        if ( t0 > t1 ) swap(t0, t1);

        if ( t0 < 0 )
        {
            if ( t1 < 0 ) goto complete;
            if ( t1 < adist )
            {
                float z = O.z + t1 * D.z;
                if ( abs(z) <= mHalfHeight )
                {
                    ts = t1;
                    sResult = IR_HITINSIDE;
                    goto complete;
                }
            }
        }
        else
        {
            if ( t0 < adist )
            {
                float z = O.z + t0 * D.z;
                if ( abs(z) <= mHalfHeight )
                {
                    ts = t0;
                    sResult = IR_HIT;
                }
            }
        }

complete:
        if ( tResult != IR_MISS )
        {
            if ( sResult != IR_MISS )
            {
                if ( tf < ts )
                {
                    _ASSERT( tf > 0 );
                    adist = tf;
                    return tResult;
                }
                else
                {
                    _ASSERT( ts > 0 );
                    adist = ts;
                    return sResult;
                }
            }
            else
            {
                _ASSERT( tf > 0 );
                adist = tf;
                return tResult;
            }
        }
        else if ( sResult != IR_MISS )
        {
            _ASSERT( ts > 0 );
            adist = ts;
            return sResult;
        }

        return IR_MISS;
    }

    Color Cylinder::GetColor(const Vector &pos)
    {
        if ( mMaterial->mTexture ) {
            // eer..
            return mMaterial->mColor;
        } else {
            return mMaterial->mColor;
        }
        
    }

}