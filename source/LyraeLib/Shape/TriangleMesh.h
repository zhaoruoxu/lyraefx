#if defined(_MSC_VER)
#pragma once
#endif

#ifndef LYRAE_SHAPE_TRIANGLEMESH_H
#define LYRAE_SHAPE_TRIANGLEMESH_H

#include "LyraeFX.h"
#include "Shape.h"


namespace LyraeFX
{
    class TriangleMesh : public Shape
    {
    public:
        TriangleMesh(Material *material, int nt, int nv, const int *vptr,
            const Vector *P, const Vector *N, const float *uv);

        virtual ~TriangleMesh();
        IntersectResult Intersect(const Ray &ray, float &dist);
        Vector GetNormal(const Vector &pos);
        ShapeType GetType() { return SHAPE_TRIANGLEMESH; }
        bool IntersectBox(const AABB &ab);
        AABB GetAABB();
        Color GetColor(const Vector &pos);
        Vector GetCentroid();
        bool CanIntersect() { return false; }

        int ntris, nverts;
        int *vertexIndex;
        Vector *p;
        Vector *n;
        float *uvs;
        AABB bounds;
    };

    class Triangle : public Shape
    {
    public:
        Triangle(Material *material, TriangleMesh *m, int n);
        virtual ~Triangle() {}
        IntersectResult Intersect(const Ray &ray, float &dist);
        Vector GetNormal(const Vector &pos);
        ShapeType GetType() { return SHAPE_TRIANGLE; }
        bool IntersectBox(const AABB &ab);
        AABB GetAABB();
        Color GetColor(const Vector &pos);
        Vector GetCentroid();

        TriangleMesh *mesh;
        int *v;
    protected:
        Vector mN;
        float mU, mV;
        float nu, nv, nd;
        int k;
        float bnu, bnv;
        float cnu, cnv;
    };
}

#endif // LYRAE_SHAPE_TRIANGLEMESH_H