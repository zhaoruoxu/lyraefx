#if defined(_MSC_VER)
#pragma once
#endif

#ifndef LYRAE_CORE_GEOMETRY_H
#define LYRAE_CORE_GEOMETRY_H

#include "LyraeFX.h"

namespace LyraeFX
{
    inline void Clamp(float &x, float a, float b)
    {
        if ( x < a ) x = a;
        if ( x > b ) x = b;
    }

    inline float Radians(float deg) 
    {
        return ((float)M_PI/180.f) * deg;
    }

    inline float Degrees(float rad) 
    {
        return (180.f/(float)M_PI) * rad;
    }

    class Vector
    {
    public:
        Vector() { x = y = z = 0.f; }
        Vector(float xx, float yy, float zz)
            :x(xx), y(yy), z(zz)
        {
            _ASSERT(!HasNaNs());
        }

        Vector(const Vector& v)
        {
            _ASSERT(!v.HasNaNs());
            x = v.x; y = v.y; z = v.z;
        }
        Vector &operator=(const Vector &v)
        {
            _ASSERT(!v.HasNaNs());
            x = v.x; y = v.y; z = v.z;
            return *this;
        }

		bool HasNaNs() const 
		{
			return isnan(x) || isnan(y) || isnan(z);
		}

        friend Vector operator*(const Vector &v1, const Vector &v2)
        {
            _ASSERT(!v1.HasNaNs());
            _ASSERT(!v2.HasNaNs());
            return Vector(v1.x * v2.x, v1.y * v2.y, v1.z * v2.z);
        }
        
        friend Vector operator*(const Vector &v, float f)
        {
            _ASSERT(!v.HasNaNs());
            _ASSERT(!isnan(f));
            return Vector(v.x * f, v.y * f, v.z * f);
        }
        friend Vector operator*(float f, const Vector &v)
        {
            _ASSERT(!isnan(f));
            _ASSERT(!v.HasNaNs());
            return Vector(v.x * f, v.y * f, v.z * f);
        }
        friend Vector operator+(const Vector &v1, const Vector &v2)
        {
            _ASSERT(!v1.HasNaNs());
            _ASSERT(!v2.HasNaNs());
            return Vector(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);
        }
        friend Vector operator-(const Vector &v1, const Vector &v2)
        {
            _ASSERT(!v1.HasNaNs());
            _ASSERT(!v2.HasNaNs());
            return Vector(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
        }
        friend Vector operator/(const Vector &v1, const Vector &v2)
        {
            _ASSERT(!v1.HasNaNs());
            _ASSERT(!v2.HasNaNs());
            _ASSERT( v2.x != 0 );
            _ASSERT( v2.y != 0 );
            _ASSERT( v2.z != 0 );
            return Vector(v1.x / v2.x, v1.y / v2.y, v1.z / v2.z);
        }
        friend Vector operator/(const Vector &v1, float f)
        {
            _ASSERT( !v1.HasNaNs() );
            _ASSERT( !isnan(f) );
            _ASSERT( f != 0 );
            float inv = 1.f / f;
            return Vector(v1.x * inv, v1.y * inv, v1.z * inv);
        }
        
        Vector operator-() const
        {
            return Vector(-x, -y, -z);
        }
        Vector &operator+=(const Vector &v)
        {
            _ASSERT( !v.HasNaNs() );
            x += v.x; y += v.y; z += v.z;
            return *this;
        }
        Vector &operator-=(const Vector &v)
        {
            _ASSERT( !v.HasNaNs() );
            x -= v.x; y -= v.y; z -= v.z;
            return *this;
        }
        Vector &operator*=(float f)
        {
            x *= f; y *= f; z *= f;
            return *this;
        }
        Vector &operator*=(const Vector &v)
        {
            _ASSERT( !v.HasNaNs() );
            x *= v.x; y *= v.y; z *= v.z;
            return *this;
        }
        Vector &operator/=(float f)
        {
            _ASSERT( f != 0 );
            float inv = 1.f / f;
            x *= inv; y *= inv; z *= inv;
            return *this;
        }
        bool operator==(const Vector &v) const
        {
            _ASSERT( !v.HasNaNs() );
            return x == v.x && y == v.y && z == v.z;
        }
        bool operator!=(const Vector &v) const
        {
            _ASSERT( !v.HasNaNs() );
            return x != v.x || y != v.y || z != v.z;
        }

        float operator[](int i) const
        {
            _ASSERT(i >= 0 && i <= 2);
            return (&x)[i];
        }
        float &operator[](int i)
        {
            _ASSERT(i >= 0 && i <= 2);
            return (&x)[i];
        }

        float Luminance()
        {
            return 0.299f * x + 0.587f * y + 0.114f * z;
        }

        void Clamp(float a, float b)
        {
            LyraeFX::Clamp(x, a, b);
            LyraeFX::Clamp(y, a, b);
            LyraeFX::Clamp(z, a, b);
        }

        float LengthSquared() const
        {
#if USE_SSE_VECTOR
            float l;
            __asm
            {
                    mov         eax, this
                    movups      xmm0, [eax]
                    mulps       xmm0, xmm0
                    movaps      xmm1, xmm0
                    shufps      xmm0, xmm1, 01001110b
                    addps       xmm0, xmm1
                    movaps      xmm1, xmm0
                    shufps      xmm1, xmm1, 00010001b
                    addps       xmm0, xmm1
                    movss       l, xmm0
            }
            return l;
#else
            return x * x + y * y + z * z;
#endif
        }
        float Length() const
        {
#if USE_SSE_VECTOR
            float l;
            __asm
            {
                mov         eax, this
                    movups      xmm0, [eax]
                mulps       xmm0, xmm0
                    movaps      xmm1, xmm0
                    shufps      xmm0, xmm1, 01001110b
                    addps       xmm0, xmm1
                    movaps      xmm1, xmm0
                    shufps      xmm1, xmm1, 00010001b
                    addps       xmm0, xmm1
                    sqrtps      xmm0, xmm0
                    movss       l, xmm0
            }
            return l;
#else
            return sqrtf(LengthSquared());
#endif
        }
        Vector GetNormalizedVector() const
        {
#if USE_SSE_VECTOR
            Vector v;
            __asm
            {
                    mov         eax, this
                    movups      xmm0, [eax]
                    movaps      xmm2, xmm0
                    mulps       xmm0, xmm0
                    movaps      xmm1, xmm0
                    shufps      xmm0, xmm1, 01001110b
                    addps       xmm0, xmm1
                    movaps      xmm1, xmm0
                    shufps      xmm1, xmm1, 00010001b
                    addps       xmm0, xmm1
                    rsqrtps     xmm0, xmm0
                    mulps       xmm2, xmm0
                    movups      v, xmm2
            }
            return v;
#else
            float l = Length();
            _ASSERT( l != 0 );
            l = 1.f / l;
            return Vector(x * l, y * l, z * l);
#endif
        }
        void Normalize()
        {
#if USE_SSE_VECTOR
            __asm
            {
                mov         eax, this
                movups      xmm0, [eax]
                movaps      xmm2, xmm0
                mulps       xmm0, xmm0
                movaps      xmm1, xmm0
                shufps      xmm0, xmm1, 01001110b
                addps       xmm0, xmm1
                movaps      xmm1, xmm0
                shufps      xmm1, xmm1, 00010001b
                addps       xmm0, xmm1
                rsqrtps     xmm0, xmm0
                mulps       xmm2, xmm0
                movups      [eax], xmm2
            }
#else
            float l = Length();
            if ( l == 0 )
            {
                x = y = z = 0;
            }
            l = 1.f / l;
            x *= l; y *= l; z *= l;
#endif
        }
        float Dot(const Vector &v) const
        {
            return x * v.x + y * v.y + z * v.z;
        }
        Vector Cross(const Vector &v) const
        {
            return Vector(y * v.z - z * v.y,
                z * v.x - x * v.z,
                x * v.y - y * v.x);
        }
        Vector Powf(float a)
        {
            Vector v;
            v.x = powf(x, a);
            v.y = powf(y, a);
            v.z = powf(z, a);
            return v;
        }

        bool IsBlack();
        friend std::ostream &operator<<(std::ostream &o, const Vector &v)
        {
            o << "(" << v.x << "," << v.y << "," << v.z << ")";
            return o;
        }

        union {
            struct { float x, y, z; };
            struct { float cell[3]; };
        };
    }; // class Vector

    typedef Vector Color;

    static const Color gRed(1.f, 0.f, 0.f);
    static const Color gBlue(0.f, 0.f, 1.f);
    static const Color gGreen(0.f, 1.f, 0.f);
    static const Color gWhite(1.f, 1.f, 1.f);

    class Matrix
    {
        enum 
        { 
            TX=3, 
            TY=7, 
            TZ=11, 
            D0=0, D1=5, D2=10, D3=15, 
            SX=D0, SY=D1, SZ=D2, 
            W=D3 
        };
    public:
        Matrix() { Identity(); }

        void Rotate( const Vector &pos, float rx, float ry, float rz )
        {
            Matrix t;
            t.RotateX( rz );
            RotateY( ry );
            Concatenate( t );
            t.RotateZ( rx );
            Concatenate( t );
            Translate( pos );
        }

        void Identity()
        {
            a12 = a13 = a14 = 0.f;
            a21 = a23 = a24 = 0.f;
            a31 = a32 = a34 = 0.f;
            a41 = a42 = a43 = 0.f;
            a11 = a22 = a33 = a44 = 1.f;
        }

        void RotateX( float rx )
        {
            float sx = sinf( rx * M_PI / 180.f );
            float cx = cosf( rx * M_PI / 180.f );
            Identity();
            cell[5] = cx; cell[6] = sx; cell[9] = -sx; cell[10] = cx;
        }

        void RotateY( float ry )
        {
            float sy = sinf( ry * M_PI / 180.f );
            float cy = cosf( ry * M_PI / 180.f );
            Identity();
            cell[0] = cy; cell[2] = -sy; cell[8] = sy; cell[10] = cy;
        }

        void RotateZ( float rz )
        {
            float sz = sinf( rz * M_PI / 180.f );
            float cz = cosf( rz * M_PI / 180.f );
            Identity();
            cell[0] = cz; cell[1] = sz; cell[4] = -sz; cell[5] = cz;
        }

        void Translate( Vector pos )
        {
            cell[TX] += pos.x;
            cell[TY] += pos.y;
            cell[TZ] += pos.z;
        }

        void Concatenate( Matrix &m )
        {
            Matrix res;
            for ( int c = 0; c < 4; c++ ) for ( int r = 0; r < 4; r++ )
                res.cell[r * 4 + c] = cell[r * 4] * m.cell[c] +
                cell[r * 4 + 1] * m.cell[c + 4] +
                cell[r * 4 + 2] * m.cell[c + 8] +
                cell[r * 4 + 3] * m.cell[c + 12];
            for ( int c = 0; c < 16; c++ ) cell[c] = res.cell[c];
        }

        Vector Transform( const Vector &v )
        {
            float x = a11 * v.x + a12 * v.y + a13 * v.z + a14;
            float y = a21 * v.x + a22 * v.y + a23 * v.z + a24;
            float z = a31 * v.x + a32 * v.y + a33 * v.z + a34;
            return Vector( x, y, z );
        }

        void Invert()
        {
            Matrix t;
            float tx = -cell[3], ty = -cell[7], tz = -cell[11];
            for ( int h = 0; h < 3; h++ ) for ( int v = 0; v < 3; v++ ) t.cell[h + v * 4] = cell[v + h * 4];
            for ( int i = 0; i < 11; i++ ) cell[i] = t.cell[i];
            cell[3] = tx * cell[0] + ty * cell[1] + tz * cell[2];
            cell[7] = tx * cell[4] + ty * cell[5] + tz * cell[6];
            cell[11] = tx * cell[8] + ty * cell[9] + tz * cell[10];
        }

        union
        {
            struct 
            { 
                float      a11, a12, a13, a14,
                           a21, a22, a23, a24,
                           a31, a32, a33, a34,
                           a41, a42, a43, a44;
            };

            struct 
            {
                float cell[16];
            };
        };
    };


    class Ray
    {
    public:
        Ray() : mOrigin(Vector(0, 0, 0)), mDirection(Vector(0, 0, 0))
        {
        };
        Ray(const Vector &origin, const Vector &direction)
            :mOrigin(origin), mDirection(direction.GetNormalizedVector())
        {
        }

        Vector mOrigin;
        Vector mDirection;
    }; // class Ray

    class Pln
    {
    public:
        Pln() : N(Vector(0, 0, 0)), D(0.f) {}
        Pln(const Vector &n, float d)
            :N(n), D(d)
        {
        }

        Vector N;
        float D;
    }; // class Pln

    class AABB
    {
    public:
        AABB() : mPos(INFINITY * 0.5f, INFINITY * 0.5f, INFINITY * 0.5f), 
            mSize(-INFINITY, -INFINITY, -INFINITY) {}
        AABB(const Vector &pos, const Vector &size)
            :mPos(pos), mSize(size) {}

        AABB(const AABB &ab)
            :mPos(ab.mPos), mSize(ab.mSize) {}

        bool Intersect(const AABB &b);

        IntersectResult Intersect(const Ray &ray, float &adist);

        bool Contains(const Vector &pos);

        float SurfaceArea() const
        {
            return 2.f * (mSize.x * mSize.y + mSize.x * mSize.z + mSize.y * mSize.z);
        }

        Vector operator[](int i) const
        {
            _ASSERT(i == 0 || i == 1);
            if ( i == 0 ) return mPos;
            else return mPos + mSize;
        }

        Vector GetCentroid() const
        {
            return mPos + 0.5f * mSize;
        }
        int32_t MaximumExtent() const
        {
            if ( mSize.x > mSize.y && mSize.x > mSize.z )
            {
                return 0;
            }
            else if ( mSize.y > mSize.z )
            {
                return 1;
            }
            else
            {
                return 2;
            }
        }

        Vector mPos, mSize;
    }; // class AABB

    AABB Union(const AABB &a, const Vector &p);
    AABB Union(const AABB &a, const AABB &b);

    inline bool IntersectBoxP(const AABB &bounds, const Ray &ray, 
        const Vector &invDir, const uint32_t dirIsNeg[3], float dist)
    {
        float tmin =  (bounds[  dirIsNeg[0]].x - ray.mOrigin.x) * invDir.x;
        float tmax =  (bounds[1-dirIsNeg[0]].x - ray.mOrigin.x) * invDir.x;
        float tymin = (bounds[  dirIsNeg[1]].y - ray.mOrigin.y) * invDir.y;
        float tymax = (bounds[1-dirIsNeg[1]].y - ray.mOrigin.y) * invDir.y;

        if ((tmin > tymax) || (tymin > tmax))
            return false;
        if (tymin > tmin) tmin = tymin;
        if (tymax < tmax) tmax = tymax;

        float tzmin = (bounds[  dirIsNeg[2]].z - ray.mOrigin.z) * invDir.z;
        float tzmax = (bounds[1-dirIsNeg[2]].z - ray.mOrigin.z) * invDir.z;
        if ((tmin > tzmax) || (tzmin > tmax))
            return false;
        if (tzmin > tmin)
            tmin = tzmin;
        if (tzmax < tmax)
            tmax = tzmax;
        return (tmin < dist) && (tmax > 0);
    }

    inline bool Quadratic(float A, float B, float C, float *t0, float *t1) {
        // Find quadratic discriminant
        float discrim = B * B - 4.f * A * C;
        if (discrim <= 0.) return false;
        float rootDiscrim = sqrtf(discrim);

        // Compute quadratic _t_ values
        float q;
        if (B < 0) q = -.5f * (B - rootDiscrim);
        else       q = -.5f * (B + rootDiscrim);
        *t0 = q / A;
        *t1 = C / q;
        if (*t0 > *t1) swap(*t0, *t1);
        return true;
    }

    inline void Clamp(Vector &x, const Vector &a, const Vector &b)
    {
        Clamp(x.x, a.x, b.x);
        Clamp(x.y, a.y, b.y);
        Clamp(x.z, a.z, b.z);
    }
} // namespace LyraeFX

#endif // LYRAE_CORE_GEOMETRY_H