#if defined(_MSC_VER)
#pragma once
#endif

#ifndef LYRAE_CORE_SHAPE_H
#define LYRAE_CORE_SHAPE_H

#include "LyraeFX.h"
#include "Material.h"
#include "Geometry.h"

namespace LyraeFX
{

    typedef enum _ShapeType
    {
        SHAPE_ABSTRACT,
        SHAPE_SPHERE,
        SHAPE_PLANE,
        SHAPE_CUBE,
        SHAPE_BOX,
        SHAPE_CYLINDER,
        SHAPE_TCYLINDER,
        SHAPE_TRIANGLEMESH,
        SHAPE_TRIANGLE
    } ShapeType;



    class Shape
    {
        static uint32_t mID;
    public:
        Shape() : mMaterial(NULL), mShapeID(mID++) {}
        Shape(Material *material)
            :mMaterial(material), mShapeID(mID++)
        {
        }
        virtual ~Shape() {};
        virtual IntersectResult Intersect(const Ray &ray, float &dist) = 0;
        virtual Vector GetNormal(const Vector &pos) = 0;
        virtual Color GetColor(const Vector &pos) { return mMaterial->mColor; };
        virtual ShapeType GetType() = 0;
        virtual bool IntersectBox(const AABB &ab) = 0;
        virtual AABB GetAABB() = 0;
        virtual Vector GetCentroid() = 0;
        virtual bool CanIntersect() { return true; }

        Material *mMaterial;
        const uint32_t mShapeID;
        bool mVisible;

    };// class Shape


} // namespace LyraeFX


#endif // LYRAE_CORE_SHAPE_H