#include "BVH.h"
#include "../Config/EngineConfig.h"

namespace LyraeFX
{
    struct CompareToMid
    {
        CompareToMid(int d, float m) { dim = d; mid = m; }
        bool operator()(const BVHShapeInfo &a) const
        {
            return a.centroid[dim] < mid;
        }
        int dim;
        float mid;
    };

    struct ComparePoints
    {
        ComparePoints(int d) { dim = d; }
        int dim;
        bool operator()(const BVHShapeInfo &a, const BVHShapeInfo &b) const
        {
            return a.centroid[dim] < b.centroid[dim];
        }
    };

    struct CompareToBucket {
        CompareToBucket(int split, int num, int d, const AABB &b)
            : centroidBounds(b)
        { splitBucket = split; nBuckets = num; dim = d; }
        bool operator()(const BVHShapeInfo &p) const
        {
            int b = (int) (nBuckets * ((p.centroid[dim] - centroidBounds.mPos[dim]) /
                (centroidBounds.mSize[dim])));
            if (b == nBuckets) b = nBuckets-1;
            _ASSERT(b >= 0 && b < nBuckets);
            return b <= splitBucket;
        }

        int splitBucket, nBuckets, dim;
        const AABB &centroidBounds;
    };



    BVHAccelerator::BVHAccelerator(const vector<Shape *> &p, uint32_t ms, SplitMethod m /* = BVH_EQUAL_COUNTS */)
    {
        maxShapesInNode = min(255, ms);
        splitMethod = m;
        for ( uint32_t i = 0; i < p.size(); i++ )
        {
            if ( p[i]->CanIntersect() )
                shapes.push_back(p[i]);
        }

        if ( shapes.size() == 0 )
        {
            return;
        }
        vector<BVHShapeInfo> buildData;
        buildData.reserve(p.size());
        for ( uint32_t i = 0; i < shapes.size(); i++ )
        {
            AABB ab = shapes[i]->GetAABB();
            buildData.push_back(BVHShapeInfo(i, ab));
        }

        int32_t totalNodes = 0;
        vector<Shape *> orderedShapes;
        orderedShapes.reserve(shapes.size());
        root = recursiveBuild(buildData, 0, shapes.size(), &totalNodes, orderedShapes);
        shapes.swap(orderedShapes);
        nodes = new LinearBVHNode[totalNodes];
        uint32_t offset = 0;
        flattenBVHTree(root, &offset);
        _ASSERT(offset == totalNodes);
        ClearTree(root);
        root = NULL;
    }

    BVHNode *BVHAccelerator::recursiveBuild(vector<BVHShapeInfo> &buildData, uint32_t start, uint32_t end,
        int32_t *totalNodes, vector<Shape *> &orderedShapes)
    {
        _ASSERT(start != end);
        (*totalNodes)++;
        BVHNode *node = new BVHNode();
        AABB bbox;
        for ( uint32_t i = start; i < end; i++ )
        {
            bbox = Union(bbox, buildData[i].bounds);
        }
        uint32_t nShapes = end - start;
        if ( nShapes == 1 )
        {
            uint32_t firstShapeOffset = orderedShapes.size();
            for ( uint32_t i = start; i < end; i++ )
            {
                uint32_t shapeNum = buildData[i].shapeNumber;
                orderedShapes.push_back(shapes[shapeNum]);
            }
            node->InitLeaf(firstShapeOffset, nShapes, bbox);
            return node;
        }

        AABB centroidBounds;
        for ( uint32_t i = start; i < end; i++ )
        {
            centroidBounds = Union(centroidBounds, buildData[i].centroid);
        }
        int dim = centroidBounds.MaximumExtent();

        uint32_t mid = (start + end) / 2;
        if ( centroidBounds.mSize[dim] == 0 )
        {
            uint32_t firstShapeOffset = orderedShapes.size();
            for ( uint32_t i = start; i < end; i++ )
            {
                uint32_t shapeNum = buildData[i].shapeNumber;
                orderedShapes.push_back(shapes[shapeNum]);
            }
            node->InitLeaf(firstShapeOffset, nShapes, bbox);
            return node;
        }

        switch (splitMethod)
        {
        case BVH_MIDDLE:
            {
                float pmid = centroidBounds.mPos[dim] + centroidBounds.mSize[dim] * 0.5f;
                BVHShapeInfo *midPtr = std::partition(&buildData[start], &buildData[end - 1] + 1,
                    CompareToMid(dim, pmid));
                mid = midPtr - &buildData[0];
                break;
            }
        case BVH_EQUAL_COUNTS:
            {
                mid = (start + end) / 2;
                std::nth_element(&buildData[start], &buildData[mid],
                                 &buildData[end - 1] + 1, ComparePoints(dim));
                break;
            }
        case BVH_SAH: default:
            {
                if ( nShapes <= 4 )
                {
                    mid = (start + end) / 2;
                    std::nth_element(&buildData[start], &buildData[mid],
                        &buildData[end - 1] + 1, ComparePoints(dim));
                    break;
                }
                
                const int nBuckets = 12;
                struct BucketInfo {
                    BucketInfo() { count = 0; }
                    int count;
                    AABB bounds;
                };
                BucketInfo buckets[nBuckets];

                for (uint32_t i = start; i < end; ++i) 
                {
                    int b = (int) (nBuckets *
                        ((buildData[i].centroid[dim] - centroidBounds.mPos[dim]) /
                        (centroidBounds.mSize[dim])));
                    if (b == nBuckets) b = nBuckets-1;
                    _ASSERT(b >= 0 && b < nBuckets);
                    buckets[b].count++;
                    buckets[b].bounds = Union(buckets[b].bounds, buildData[i].bounds);
                }

                float cost[nBuckets-1];
                for (int i = 0; i < nBuckets-1; ++i) 
                {
                    AABB b0, b1;
                    int count0 = 0, count1 = 0;
                    for (int j = 0; j <= i; ++j) 
                    {
                        b0 = Union(b0, buckets[j].bounds);
                        count0 += buckets[j].count;
                    }
                    for (int j = i+1; j < nBuckets; ++j) 
                    {
                        b1 = Union(b1, buckets[j].bounds);
                        count1 += buckets[j].count;
                    }
                    cost[i] = .125f + (count0*b0.SurfaceArea() + count1*b1.SurfaceArea()) /
                        bbox.SurfaceArea();
                }

                float minCost = cost[0];
                uint32_t minCostSplit = 0;
                for (int i = 1; i < nBuckets-1; ++i) 
                {
                    if (cost[i] < minCost) 
                    {
                        minCost = cost[i];
                        minCostSplit = i;
                    }
                }

                if (nShapes > maxShapesInNode || minCost < nShapes) 
                {
                    BVHShapeInfo *pmid = std::partition(&buildData[start],
                        &buildData[end-1]+1,
                        CompareToBucket(minCostSplit, nBuckets, dim, centroidBounds));
                    mid = pmid - &buildData[0];
                }
                else {
                    uint32_t firstPrimOffset = orderedShapes.size();
                    for (uint32_t i = start; i < end; ++i) {
                        uint32_t primNum = buildData[i].shapeNumber;
                        orderedShapes.push_back(shapes[primNum]);
                    }
                    node->InitLeaf(firstPrimOffset, nShapes, bbox);
                }
                break;
            }
        }

        node->InitInterior(dim,
            recursiveBuild(buildData, start, mid, totalNodes, orderedShapes),
            recursiveBuild(buildData, mid, end, totalNodes, orderedShapes));

        return node;
    }

    uint32_t BVHAccelerator::flattenBVHTree(BVHNode *node, uint32_t *offset)
    {
        LinearBVHNode *linearNode = &nodes[*offset];
        linearNode->bounds = node->bounds;
        uint32_t myOffset = (*offset)++;
        if (node->nShapes > 0) {
            _ASSERT(!node->children[0] && !node->children[1]);
            linearNode->shapesOffset = node->firstShapeOffset;
            linearNode->nShapes = node->nShapes;
        }
        else {
            // Creater interior flattened BVH node
            linearNode->axis = node->splitAxis;
            linearNode->nShapes = 0;
            flattenBVHTree(node->children[0], offset);
            linearNode->secondChildOffset = flattenBVHTree(node->children[1],
                offset);
        }
        return myOffset;
    }

    IntersectResult BVHAccelerator::FindNearest(const Ray &ray, float &dist, Shape *&shape)
    {
        if ( !nodes ) return IR_MISS;
        IntersectResult iresult = IR_MISS;
        Vector origin = ray.mOrigin;
        Vector invDir(1.f / ray.mDirection.x, 1.f / ray.mDirection.y, 1.f / ray.mDirection.z);
        uint32_t dirIsNeg[3] = { invDir.x < 0, invDir.y < 0, invDir.z < 0 };
        uint32_t todoOffset = 0, nodeNum = 0;
        uint32_t todo[64];

        while ( true )
        {
            const LinearBVHNode *node = &nodes[nodeNum];
            float bdist = MAXDISTANCE;
            if ( IntersectBoxP(node->bounds, ray, invDir, dirIsNeg, bdist) )
            {
                if ( node->nShapes > 0 )
                {
                    for ( uint32_t i = 0; i < node->nShapes; i++ )
                    {
                        float ldist = dist;
                        IntersectResult ir = shapes[node->shapesOffset + i]->Intersect(ray, ldist);
                        if ( ir != IR_MISS && ldist < dist )
                        {
                            iresult = ir;
                            dist = ldist;
                            shape = shapes[node->shapesOffset + i];
                        }
                    }
                    if ( todoOffset == 0 ) break;
                    nodeNum = todo[--todoOffset];
                }
                else
                {
                    if ( dirIsNeg[node->axis] )
                    {
                        todo[todoOffset++] = nodeNum + 1;
                        nodeNum = node->secondChildOffset;
                    }
                    else
                    {
                        todo[todoOffset++] = node->secondChildOffset;
                        nodeNum++;
                    }
                }
            }
            else
            {
                if ( todoOffset == 0 ) break;
                nodeNum = todo[--todoOffset];
            }
        }

        return iresult;

    }

    IntersectResult BVHAccelerator::FindNearestNode(BVHNode *node, const Ray &ray, float &dist, Shape *&shape)
    {
        _ASSERT(node);
        IntersectResult iresult = IR_MISS;
        if ( node->children[0] == NULL )
        {
            _ASSERT(node->children[1] == NULL);
            
            for ( int32_t i = node->firstShapeOffset; i < node->firstShapeOffset + node->nShapes; i++ )
            {
                float ldist = dist;
                IntersectResult ir = shapes[i]->Intersect(ray, ldist);
                if ( ir != IR_MISS && ldist < dist )
                {
                    dist = ldist;
                    iresult = ir;
                    shape = shapes[i];
                }
            }
            return iresult;
        }
        else
        {
            _ASSERT(node->children[1]);
            Vector invDir(1.f / ray.mDirection.x, 1.f / ray.mDirection.y, 1.f / ray.mDirection.z);
            uint32_t dirIsNeg[3] = { invDir.x < 0, invDir.y < 0, invDir.z < 0 };
            float bdist = MAXDISTANCE;
            if ( IntersectBoxP(node->children[0]->bounds, ray, invDir, dirIsNeg, bdist) )
            {
                float ldist = dist;
                IntersectResult ir = FindNearestNode(node->children[0], ray, ldist, shape);
                if ( ir != IR_MISS && ldist < dist )
                {
                    dist = ldist;
                    iresult = ir;
                }
            }
            bdist = MAXDISTANCE;
            if ( IntersectBoxP(node->children[1]->bounds, ray, invDir, dirIsNeg, bdist) )
            {
                float ldist = dist;
                IntersectResult ir = FindNearestNode(node->children[1], ray, ldist, shape);
                if ( ir != IR_MISS && ldist < dist )
                {
                    dist = ldist;
                    iresult = ir;
                }
            }
        }
        return iresult;
    }
}