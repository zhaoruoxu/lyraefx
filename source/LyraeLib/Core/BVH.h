#if defined(_MSC_VER)
#pragma once
#endif

#ifndef LYRAE_CORE_BVH_H
#define LYRAE_CORE_BVH_H

#include "LyraeFX.h"
#include "Shape.h"
#include "Geometry.h"


namespace LyraeFX
{
    enum SplitMethod
    {
        BVH_MIDDLE,
        BVH_EQUAL_COUNTS,
        BVH_SAH,
    };

    struct BVHShapeInfo
    {
        BVHShapeInfo(int n, const AABB &ab)
            :shapeNumber(n), centroid(ab.GetCentroid()), bounds(ab)
        {
        }

        int shapeNumber;
        Vector centroid;
        AABB bounds;
    };

    struct BVHNode
    {
        BVHNode() { children[0] = children[1] = NULL; }
        void InitLeaf(int32_t first, int32_t n, const AABB &ab)
        {
            firstShapeOffset = first;
            nShapes = n;
            bounds = ab;
        }
        void InitInterior(int32_t axis, BVHNode *c0, BVHNode *c1)
        {
            _ASSERT(c0);
            _ASSERT(c1);
            children[0] = c0;
            children[1] = c1;
            bounds = Union(c0->bounds, c1->bounds);
            splitAxis = axis;
            nShapes = 0;
        }
        AABB bounds;
        BVHNode *children[2];
        int32_t splitAxis, firstShapeOffset, nShapes;
    };

    struct LinearBVHNode {
        AABB bounds;
        union {
            uint32_t shapesOffset;
            uint32_t secondChildOffset;
        };

        uint8_t nShapes;
        uint8_t axis;
        uint8_t pad[2];
    };

    class BVHAccelerator
    {
    public:
        BVHAccelerator(const vector<Shape *> &p, uint32_t maxShapes = 1, SplitMethod m = BVH_SAH);

        virtual ~BVHAccelerator()
        {
            delete [] nodes;
        }
        BVHNode *recursiveBuild(vector<BVHShapeInfo> &buildData, uint32_t start, uint32_t end,
            int32_t *totalNodes, vector<Shape *> &orderedShapes);
        uint32_t flattenBVHTree(BVHNode *node, uint32_t *offset);
        IntersectResult FindNearest(const Ray &ray, float &dist, Shape *&shape);
        IntersectResult FindNearestNode(BVHNode *node, const Ray &ray, float &dist, Shape *&shape);

        void ClearTree(BVHNode *r)
        {
            if ( !r ) return;
            if ( r->children[0] )
                ClearTree(r->children[0]);
            if ( r->children[1] )
                ClearTree(r->children[1]);
            delete r;
        }


        uint32_t maxShapesInNode;
        SplitMethod splitMethod;
        vector<Shape *> shapes;
        BVHNode *root;
        LinearBVHNode *nodes;
    };
}


#endif // LYRAE_CORE_BVH_H