
#if defined(_MSC_VER)
#pragma once
#endif

#ifndef LYRAE_SHAPE_SPHERE_H
#define LYRAE_SHAPE_SPHERE_H

#include "../Core/LyraeFX.h"
#include "../Core/Shape.h"

namespace LyraeFX
{

    class Sphere : public Shape
    {
    public:
        Sphere(Material *material, const Vector &center, float radius)
            :Shape(material), mCenter(center), mRadius(radius), mSquaredRadius(radius * radius),
            mReverseRadius(1.f / radius)
        {
        }
        virtual ~Sphere() {}
        IntersectResult Intersect(const Ray &ray, float &dist);
        Vector GetNormal(const Vector &pos)
        {
            return (pos - mCenter).GetNormalizedVector();
        }
        ShapeType GetType() { return SHAPE_SPHERE; }
        bool IntersectBox(const AABB &ab);
        AABB GetAABB()
        {
            Vector size(mRadius, mRadius, mRadius);
            return AABB(mCenter - size, size * 2.f);
        }
        Color GetColor(const Vector &pos);
        Vector GetCentroid()
        {
            return mCenter;
        }
        bool CanIntersect() { return true; }

        Vector mCenter;
        float mRadius;
        float mSquaredRadius;
        float mReverseRadius;

    }; // class Sphere


} // namespace LyraeFX

#endif // LYRAE_SHPAE_SPHERE_H