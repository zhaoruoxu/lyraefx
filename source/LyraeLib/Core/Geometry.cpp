#include "LyraeFX.h"
#include "Geometry.h"
#include "../Config/EngineConfig.h"

using namespace LyraeFX;

namespace LyraeFX
{
    bool Vector::IsBlack()
    {
        float e = EPSILON;
        return (abs(x) < e) && (abs(y) < e) && (abs(z) < e);
    }
    bool AABB::Intersect(const AABB &b)
    {
        Vector v1 = b.mPos, v2 = b.mPos + b.mSize;
        Vector v3 = mPos, v4 = mPos + mSize;
        return (v4.x > v1.x) && (v3.x < v2.x) &&
            (v4.y > v1.y) && (v3.y < v2.y) &&
            (v4.z > v1.z) && (v3.z < v2.z);
    }

    bool AABB::Contains(const Vector &pos)
    {
        Vector v1 = mPos, v2 = mPos + mSize;
        float e = EPSILON;
        return (pos.x > v1.x - e) && (pos.x < v2.x + e) &&
            (pos.y > v1.y - e) && (pos.y < v2.y + e) &&
            (pos.z > v1.z - e) && (pos.z < v2.z + e);
    }

    IntersectResult AABB::Intersect(const Ray &ray, float &adist)
    {
        float dist[6];
        Vector ip[6], d = ray.mDirection, o = ray.mOrigin;
        IntersectResult retval = IR_MISS;

        for ( int i = 0; i < 6; i++ ) dist[i] = -1;
        Vector v1 = mPos, v2 = mPos + mSize;
        if ( d.x != 0.f )
        {
            float rc = 1.f / d.x;
            dist[0] = (v1.x - o.x) * rc;
            dist[3] = (v2.x - o.x) * rc;
        }
        if ( d.y != 0.f )
        {
            float rc = 1.f / d.y;
            dist[1] = (v1.y - o.y) * rc;
            dist[4] = (v2.y - o.y) * rc;
        }
        if ( d.z != 0.f )
        {
            float rc = 1.f / d.z;
            dist[2] = (v1.z - o.z) * rc;
            dist[5] = (v2.z - o.z) * rc;
        }

        const float e = 0.001f;
        for ( int i = 0; i < 6; i++ ) if ( dist[i] > 0 )
        {
            ip[i] = o + dist[i] * d;
            if ( Contains(ip[i]) )
            {
                if ( dist[i] < adist )
                {
                    adist = dist[i];
                    retval = IR_HIT;
                }
            }
        }
        if ( retval == IR_HIT && Contains(o) )
        {
            retval = IR_HITINSIDE;
        }

        return retval;
    }

    AABB Union(const AABB &a, const Vector &p)
    {
        Vector minp, maxp;
        Vector amax = a.mPos + a.mSize;
        minp.x = min(a.mPos.x, p.x);
        minp.y = min(a.mPos.y, p.y);
        minp.z = min(a.mPos.z, p.z);
        maxp.x = max(amax.x, p.x);
        maxp.y = max(amax.y, p.y);
        maxp.z = max(amax.z, p.z);
        return AABB(minp, maxp - minp);
    }

    AABB Union(const AABB &a, const AABB &b)
    {
        Vector minp, maxp;
        Vector amax = a.mPos + a.mSize;
        Vector bmax = b.mPos + b.mSize;
        minp.x = min(a.mPos.x, b.mPos.x);
        minp.y = min(a.mPos.y, b.mPos.y);
        minp.z = min(a.mPos.z, b.mPos.z);
        maxp.x = max(amax.x, bmax.x);
        maxp.y = max(amax.y, bmax.y);
        maxp.z = max(amax.z, bmax.z);
        return AABB(minp, maxp - minp);
    }


} // namespace LyraeFX