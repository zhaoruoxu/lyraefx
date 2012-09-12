#if defined(_MSC_VER)
#pragma once
#endif

#ifndef LYRAE_SHAPE_BOX_H
#define LYRAE_SHAPE_BOX_H

#include "../Core/LyraeFX.h"
#include "../Core/Shape.h"

namespace LyraeFX
{
    class Box : public Shape
    {
    public:
        Box(Material *material, const AABB &ab) 
            :Shape(material), mBox(ab) {}
        Box(Material *material, const Vector &pos, const Vector &size)
            :Shape(material), mBox(pos, size) {}
        

        IntersectResult Intersect(const Ray &ray, float &dist)
        {
            return mBox.Intersect(ray, dist);
        }
        Vector GetNormal(const Vector &pos);
        ShapeType GetType() { return SHAPE_BOX; }
        bool IntersectBox(const AABB &ab) { return mBox.Intersect(ab); }
        AABB GetAABB() { return mBox; }
        Color GetColor(const Vector &pos);
        Vector GetCentroid()
        {
            return mBox.mPos + mBox.mSize * 0.5f;
        }

        AABB mBox;

    }; // class Box

} // namespace LyraeFX

#endif // LYRAE_SHAPE_BOX_H