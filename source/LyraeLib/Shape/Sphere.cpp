
#include "Sphere.h"


namespace LyraeFX
{

    IntersectResult Sphere::Intersect(const Ray &ray, float &dist)
    {
        assert(dist >= 0.f);

        Vector v = ray.mOrigin - mCenter;
        float b = -v.Dot(ray.mDirection);
        float det = b * b - v.Dot(v) + mSquaredRadius;
        IntersectResult retval = IR_MISS;
        if ( det > 0 )
        {
            det = sqrtf(det);
            float i1 = b - det;
            float i2 = b + det;
            if ( i2 > 0 )
            {
                if ( i1 < 0 )
                {
                    if ( i2 < dist )
                    {
                        dist = i2;
                        retval = IR_HITINSIDE;
                    }
                }
                else
                {
                    if ( i1 < dist )
                    {
                        dist = i1;
                        retval = IR_HIT;
                    }
                }
            }
        } 
        return retval;
    }

    bool Sphere::IntersectBox(const AABB &ab)
    {
        float dmin = 0.f;
        Vector v1 = ab.mPos, v2 = ab.mPos + ab.mSize;
        if ( mCenter.x < v1.x )
        {
            dmin += (mCenter.x - v1.x) * (mCenter.x - v1.x);
        }
        else if ( mCenter.x > v2.x )
        {
            dmin += (mCenter.x - v2.x) * (mCenter.x - v2.x);
        }
        if ( mCenter.y < v1.y )
        {
            dmin += (mCenter.y - v1.y) * (mCenter.y - v1.y);
        }
        else if ( mCenter.y > v2.y )
        {
            dmin += (mCenter.y - v2.y) * (mCenter.y - v2.y);
        }
        if ( mCenter.z < v1.z )
        {
            dmin += (mCenter.z - v1.z) * (mCenter.z - v1.z);
        }
        else if ( mCenter.z > v2.z )
        {
            dmin += (mCenter.z - v2.z) * (mCenter.z - v2.z);
        }

        return dmin <= mSquaredRadius;
    }

    Color Sphere::GetColor(const Vector &pos)
    {
        if( mMaterial->mTexture )
        {

            float x = pos.x, y = pos.y, z = pos.z;
            float u1 = acosf(y / sqrtf(x * x + y * y + 0.000001f)) * INV_PI;
            if ( x < 0 ) u1 = - u1;
            float u = (u1 + 1.f) * 0.5f;
            float v = acosf(z / pos.Length()) * INV_PI;
            return mMaterial->mTexture->GetTexel(u, v) * mMaterial->mColor;
        }
        else
        {
            return mMaterial->mColor;
        }

    }

} // namespace LyraeFX