#if defined(_MSC_VER)
#pragma once
#endif

#ifndef LYRAE_SHAPE_TCYLINDER_H
#define LYRAE_SHAPE_TCYLINDER_H

#include "LyraeFX.h"
#include "Shape.h"
#include "Geometry.h"

namespace LyraeFX
{
    class TCylinder : public Shape
    {
    public:
        TCylinder(Material *material, const Vector &center, float radius, float halfHeight)
            : Shape(material), mCenter(center), mRadius(radius), mHalfHeight(halfHeight),
            mSquaredRadius(radius * radius), mReverseRadius(1.f / radius) {}

        IntersectResult Intersect(const Ray &ray, float &adist);
        Vector GetNormal(const Vector &pos)
        {
            Vector p = pos - mCenter;
            p.z = 0;
            float d = p.LengthSquared();
            if ( d < mSquaredRadius )
                return -p.GetNormalizedVector();
            else
                return p.GetNormalizedVector();
        }
        ShapeType GetType() { return SHAPE_TCYLINDER; }
        bool IntersectBox(const AABB &ab)
        {
            return GetAABB().Intersect(ab);
        }
        AABB GetAABB()
        {
            Vector size(mRadius, mRadius, mHalfHeight);
            return AABB(mCenter - size, size * 2.f);
        }
        Color GetColor(const Vector &pos);
        Vector GetCentroid()
        {
            return mCenter;
        }

        Vector mCenter;
        float mRadius;
        float mHalfHeight;
        float mSquaredRadius;
        float mReverseRadius;
    };
}


#endif // LYRAE_SHAPE_TCYLINDER_H