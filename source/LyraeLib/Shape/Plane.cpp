
#include "Plane.h"

namespace LyraeFX
{

    IntersectResult Plane::Intersect(const Ray &ray, float &dist)
    {
        _ASSERT(dist >= 0.f);

        float d = mPlane.N.Dot(ray.mDirection);
        if ( d != 0 )
        {
            float dis = -(mPlane.N.Dot(ray.mOrigin) + mPlane.D) / d;
            if ( dis > 0 )
            {
                if ( dis < dist )
                {
                    dist = dis;
                    return IR_HIT;
                }
            }
        }
        return IR_MISS;
    }

    bool Plane::IntersectBox(const AABB &ab)
    {
        Vector v[2];
        v[0] = ab.mPos; v[1] = ab.mPos + ab.mSize;
        int side1 = 0, side2 = 0;
        for ( int i = 0; i < 8; i++ )
        {
            Vector p(v[i & 1].x, v[(i >> 1) & 1].y, v[(i >> 2) & 1].z);
            if ( p.Dot(mPlane.N) + mPlane.D < 0 )
                side1++;
            else
                side2++;
        }
        if ( side1 == 0 || side2 == 0 )
            return false;
        else
            return true;
    }

    AABB Plane::GetAABB()
    {
        float m = gEngineConfig.GetFloat("PlaneDistance");
        return AABB(Vector(-m, -m, -m), Vector(m * 2.f, m * 2.f, m * 2.f));
    }

    Color Plane::GetColor(const Vector &pos)
    {
        
        if ( mMaterial->mTexture )
        {
            Texture *t = mMaterial->mTexture;
            float u = pos.Dot(mUAxis) * mMaterial->mUScale;
            float v = pos.Dot(mVAxis) * mMaterial->mVScale;
            u -= floorf(u);
            v -= floorf(v);
            _ASSERT(u >= 0 && v >= 0);
            return t->GetTexel(u, v) * mMaterial->mColor;
        }
        else
        {
            return mMaterial->mColor;
        }
    }


} // namespace LyraeFX