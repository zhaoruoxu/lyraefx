#if defined(_MSC_VER)
#pragma once
#endif

#ifndef LYRAE_SHAPE_PLANE_H
#define LYRAE_SHAPE_PLANE_H

#include "../Core/LyraeFX.h"
#include "../Config/EngineConfig.h"
#include "../Core/Shape.h"

namespace LyraeFX
{

    class EngineConfig;
    class Plane: public Shape
    {
    public:
        Plane(Material *material, const Vector &n, float d)
            :Shape(material), mPlane(n, d)
        {
            mUAxis = Vector(mPlane.N.y, mPlane.N.z, -mPlane.N.x);
            mUAxis.Normalize();
            mVAxis = mUAxis.Cross(mPlane.N);
            mVAxis.Normalize();
        }
        virtual ~Plane() {}
        IntersectResult Intersect(const Ray &ray, float &dist);
        Vector GetNormal(const Vector &pos) { return mPlane.N; }
        ShapeType GetType() { return SHAPE_PLANE; }
        Color GetColor(const Vector &pos);

        bool IntersectBox(const AABB &ab);
        AABB GetAABB();

        Vector GetCentroid()
        {
            return -mPlane.N * mPlane.D;
        }

        Pln mPlane;
        Vector mUAxis, mVAxis;
    }; // class Plane

} // namespace LyraeFX

#endif // LYRAE_SHAPE_PLANE_H