#include "TriangleMesh.h"
#include "Material.h"
#include "Shape.h"
#include "Error.h"
#include "Material.h"

namespace LyraeFX
{
    TriangleMesh::TriangleMesh(Material *material, int nt, int nv, 
        const int *vptr, const Vector *P, const Vector *N, const float *uv)
        :Shape(material)
    {
        ntris = nt;
        nverts = nv;
        vertexIndex = new int[3 * ntris];
        memcpy(vertexIndex, vptr, 3 * ntris * sizeof(int));

        p = new Vector[nverts];
        memcpy(p, P, nverts * sizeof(Vector));

        if ( N ) {
            n = new Vector[nverts];
            memcpy(n, N, nverts * sizeof(Vector));
        } else { 
            n = NULL; 
        }
        if ( uv ) {
            uvs = new float[2 * nverts];
            memcpy(uvs, uv, 2 * nverts * sizeof(float));
        } else {
            uvs = NULL;
        }
        if ( nverts > 0 ) {
            bounds = AABB(P[0], Vector(0, 0, 0));
            for ( int i = 0; i < nverts; i++ ) {
                bounds = Union(bounds, P[i]);
            }
        }
    }

    TriangleMesh::~TriangleMesh()
    {
        if ( p ) delete [] p;
        if ( n ) delete [] n;
        if ( uvs ) delete [] uvs;
        if ( vertexIndex ) delete [] vertexIndex;
    }

    IntersectResult TriangleMesh::Intersect(const Ray &ray, float &dist)
    {
        Warning("TriangleMesh: Intersect() called!");
        return IR_MISS;
    }

    Vector TriangleMesh::GetNormal(const Vector &pos)
    {
        Severe("TriangleMesh: GetNormal() called!");
        return Vector(0, 0, 0);
    }

    bool TriangleMesh::IntersectBox(const AABB &ab)
    {
        Warning("TriangleMesh: IntersectBox() called!");
        return false;
    }

    AABB TriangleMesh::GetAABB()
    {
        return bounds;
    }

    Color TriangleMesh::GetColor(const Vector &pos)
    {
        Warning("TriangleMesh: GetColor() called!");
        return Color();
    }

    Vector TriangleMesh::GetCentroid()
    {
        return bounds.mPos + bounds.mSize * 0.5f;
    }

    Triangle::Triangle(Material *material, TriangleMesh *m, int n)
        :Shape(material)
    {
        mesh = m;
        v = &mesh->vertexIndex[3 * n];
        const Vector &A = mesh->p[v[0]];
        const Vector &B = mesh->p[v[1]];
        const Vector &C = mesh->p[v[2]];
        Vector c = B - A;
        Vector b = C - A;
        mN = -b.Cross(c);
        int u, v;
        if ( fabsf(mN.x) > fabsf(mN.y) ) {
            if ( fabsf(mN.x) > fabsf(mN.z) ) k = 0; else k = 2;
        } else {
            if ( fabsf(mN.y) > fabsf(mN.z) ) k = 1; else k = 2;
        }
        u = (k + 1) % 3;
        v = (k + 2) % 3;
        float krec = 1.f / mN.cell[k];
        nu = mN.cell[u] * krec;
        nv = mN.cell[v] * krec;
        nd = mN.Dot( A ) * krec;
        // first line equation
        float reci = 1.0f / (b.cell[u] * c.cell[v] - b.cell[v] * c.cell[u]);
        bnu = b.cell[u] * reci;
        bnv = -b.cell[v] * reci;
        // second line equation
        cnu = c.cell[v] * reci;
        cnv = -c.cell[u] * reci;
        mN.Normalize();
    }

    const unsigned int modulo[] = { 0, 1, 2, 0, 1 };
    IntersectResult Triangle::Intersect(const Ray &ray, float &dist)
    {
#define ku modulo[k + 1]
#define kv modulo[k + 2]
        Vector O = ray.mOrigin, D = ray.mDirection;
        const Vector &A = mesh->p[v[0]];
        const float lnd = 1.0f / (D.cell[k] + nu * D.cell[ku] + nv * D.cell[kv]);
        const float t = (nd - O.cell[k] - nu * O.cell[ku] - nv * O.cell[kv]) * lnd;
        if (!(dist > t && t > 0)) return IR_MISS;
        float hu = O.cell[ku] + t * D.cell[ku] - A.cell[ku];
        float hv = O.cell[kv] + t * D.cell[kv] - A.cell[kv];
        float beta = mU = hv * bnu + hu * bnv;
        if (beta < 0) return IR_MISS;
        float gamma = mV = hu * cnu + hv * cnv;
        if (gamma < 0) return IR_MISS;
        if ((mU + mV) > 1) return IR_MISS;
        dist = t;
        return (D.Dot(mN) > 0) ? IR_HITINSIDE : IR_HIT;
#undef ku
#undef kv
    }

    Vector Triangle::GetNormal(const Vector &pos)
    {
#define ku modulo[k + 1]
#define kv modulo[k + 2]
        if ( mesh->n ) {
            Vector O = pos, D = -mN;
            const Vector &A = mesh->p[v[0]];
            const float lnd = 1.0f / (D.cell[k] + nu * D.cell[ku] + nv * D.cell[kv]);
            const float t = (nd - O.cell[k] - nu * O.cell[ku] - nv * O.cell[kv]) * lnd;
            float hu = O.cell[ku] + t * D.cell[ku] - A.cell[ku];
            float hv = O.cell[kv] + t * D.cell[kv] - A.cell[kv];
            float beta = mU = hv * bnu + hu * bnv;
            float gamma = mV = hu * cnu + hv * cnv;


            const Vector &n1 = mesh->n[v[0]];
            const Vector &n2 = mesh->n[v[1]];
            const Vector &n3 = mesh->n[v[2]];
            _ASSERT(n1.LengthSquared() > EPSILON);
            _ASSERT(n2.LengthSquared() > EPSILON);
            _ASSERT(n3.LengthSquared() > EPSILON);
            float b0 = 1 - mV - mU;
            Clamp(b0, 0.f, 1.f);
            Clamp(mU, 0.f, 1.f);
            Clamp(mV, 0.f, 1.f);
            Vector n = b0 * n1 + mU * n2 + mV * n3;
            return n.GetNormalizedVector();
        } else {
            Vector e1 = mesh->p[v[1]] - mesh->p[v[0]];
            Vector e2 = mesh->p[v[2]] - mesh->p[v[0]];
            return (e1.Cross(e2)).GetNormalizedVector();
        }
#undef ku
#undef kv
    }

    bool Triangle::IntersectBox(const AABB &ab)
    {
        return GetAABB().Intersect(ab);
    }

    AABB Triangle::GetAABB()
    {
        const Vector &p1 = mesh->p[v[0]];
        const Vector &p2 = mesh->p[v[1]];
        const Vector &p3 = mesh->p[v[2]];
        float minX = min(min(p1.x, p2.x), p3.x);
        float minY = min(min(p1.y, p2.y), p3.y);
        float minZ = min(min(p1.z, p2.z), p3.z);
        float maxX = max(max(p1.x, p2.x), p3.x);
        float maxY = max(max(p1.y, p2.y), p3.y);
        float maxZ = max(max(p1.z, p2.z), p3.z);
        return AABB(Vector(minX, minY, minZ), Vector(maxX - minX, maxY - minY, maxZ - minZ));
    }

    Color Triangle::GetColor(const Vector &pos)
    {
        if ( !mMaterial->mTexture ) {
            return mMaterial->mColor;
        } else {
#define ku modulo[k + 1]
#define kv modulo[k + 2]
            Vector O = pos, D = -mN;
            const Vector &A = mesh->p[v[0]];
            const float lnd = 1.0f / (D.cell[k] + nu * D.cell[ku] + nv * D.cell[kv]);
            const float t = (nd - O.cell[k] - nu * O.cell[ku] - nv * O.cell[kv]) * lnd;
            float hu = O.cell[ku] + t * D.cell[ku] - A.cell[ku];
            float hv = O.cell[kv] + t * D.cell[kv] - A.cell[kv];
            float b1 = hv * bnu + hu * bnv;
            float b2 = hu * cnu + hv * cnv;
            float b0 = 1.f - b1 - b2;
            Clamp(b0, 0.f, 1.f);
            Clamp(b1, 0.f, 1.f);
            Clamp(b2, 0.f, 1.f);
            float u0, v0, u1, v1, u2, v2;
            if ( mesh->uvs ) {
                u0 = mesh->uvs[2*v[0]];
                v0 = mesh->uvs[2*v[0]+1];
                u1 = mesh->uvs[2*v[1]];
                v1 = mesh->uvs[2*v[1]+1];
                u2 = mesh->uvs[2*v[2]];
                v2 = mesh->uvs[2*v[2]+1];
            } else {
                u0 = 0.f; v0 = 0.f;
                u1 = 1.f; v1 = 1.f;
                u2 = 1.f; v2 = 1.f;
            }
            float u = b0 * u0 + b1 * u1 + b2 * u2;
            float v = b0 * v0 + b1 * v1 + b2 * v2;
            return mMaterial->mTexture->GetTexel(u, v) * mMaterial->mColor;
        }
#undef ku
#undef kv
    }

    Vector Triangle::GetCentroid()
    {
        const Vector &p1 = mesh->p[v[0]];
        const Vector &p2 = mesh->p[v[1]];
        const Vector &p3 = mesh->p[v[2]];
        return (p1 + p2 + p3) * 0.3333333333f;
    }
}