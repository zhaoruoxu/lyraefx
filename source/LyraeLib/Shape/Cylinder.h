#if defined(_MSC_VER)
#pragma once
#endif

#ifndef LYRAE_SHAPE_CYLINDER_H
#define LYRAE_SHAPE_CYLINDER_H

#include "LyraeFX.h"
#include "Shape.h"
#include "../Config/EngineConfig.h"


namespace LyraeFX
{
    class Cylinder : public Shape
    {
    public:
        Cylinder(Material *material, const Vector &center, float radius,
            float halfHeight)
            : Shape(material), mCenter(center), mRadius(radius), mHalfHeight(halfHeight),
            mSquaredRadius(radius * radius), mReverseRadius(1.f / radius) {}
        IntersectResult Intersect(const Ray &ray, float &adist);
        Vector GetNormal(const Vector &pos)
        {
            float d = abs(pos.z - mCenter.z);
            if ( abs(d - mHalfHeight) < EPSILON ) {
                if ( pos.z < mCenter.z )
                    return Vector(0, 0, -1);
                else
                    return Vector(0, 0, 1);
            }
            Vector p = pos - mCenter;
            p.z = 0;
            return p.GetNormalizedVector();
        }
        ShapeType GetType() { return SHAPE_CYLINDER; }
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


#endif // LYRAE_SHAPE_CYLINDER_H