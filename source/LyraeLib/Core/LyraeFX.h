#if defined(_MSC_VER)
#pragma once
#endif

#ifndef LYRAE_CORE_LYRAEFX_H
#define LYRAE_CORE_LYRAEFX_H

#pragma comment(lib, "winmm.lib")

// Global include files
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <assert.h>
#include <ostream>
#include <sstream>
#include <memory>
#include <map>
#include <deque>
#include <algorithm>
#include <Windows.h>
#include <MMSystem.h>
#include <time.h>
#include <process.h>

/* SSE optimization */
#include <xmmintrin.h>

/* DevIL */
#include "../DevIL/include/il/il.h"
#include "../DevIL/include/il/ilu.h"
#include "../DevIL/include/il/ilut.h"
#pragma comment(lib, "../LyraeLib/DevIL/lib/DevIL.lib")
#pragma comment(lib, "../LyraeLib/DevIL/lib/ILU.lib")
#pragma comment(lib, "../LyraeLib/DevIL/lib/ILUT.lib")
using std::string;
using std::vector;
using std::deque;
using std::map;
using std::auto_ptr;
using std::stringstream;
using std::swap;
using std::min;
using std::max;
using std::make_heap;
using std::pop_heap;
using std::push_heap;

#ifdef M_PI
#undef M_PI
#endif
#define M_PI       3.14159265358979323846f
#define INV_PI     0.31830988618379067154f
#define INV_TWOPI  0.15915494309189533577f
#define INV_FOURPI 0.07957747154594766788f
#ifndef INFINITY
#define INFINITY FLT_MAX
#endif
#define MAXDISTANCE     1e8f
#define EPSILON         5e-3f

#define isnan _isnan
#define isinf(f) (!_finite((f)))

#ifdef assert
#undef assert
#endif
#define assert _ASSERT


#ifdef _DEBUG
#ifdef new
#undef new
#endif // new
#define new   new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif // _DEBUG
#pragma pack(4)


const int LYRAEFX_MAJOR_VERSION = 0;
const int LYRAEFX_MINOR_VERSION = 3;
const int LYRAEFX_PATCH_VERSION = 0;

const string DefaultConfigFileName("DefaultConfig.xml");

namespace LyraeFX
{

    enum IntersectResult
    {
        IR_MISS = 0,
        IR_HIT = 1,
        IR_HITINSIDE = -1,
    };

    enum CameraType
    {
        CAMERA_PERSPECTIVE = 0,
        CAMERA_ORTHOGRAPHIC = 1,
    };

    typedef __int8 int8_t;
    typedef unsigned __int8 uint8_t;
    typedef __int16 int16_t;
    typedef unsigned __int16 uint16_t;
    typedef __int32 int32_t;
    typedef unsigned __int32 uint32_t;
    typedef __int64 int64_t;
    typedef unsigned __int64 uint64_t;
    typedef uint32_t Pixel;
    typedef volatile LONG AtomicInt32;

    class LObject;
    class Vector;
    class Ray;
    class Shape;
    class Material;
    class Sphere;
    class Pln;
    class AABB;
    class Plane;
    class Scene;
    class Engine;
    class Config;
    class EngineConfig;
    template <class X> class CountedPtr;
    class RGBRenderTarget;
    class HDRRenderTarget;
    class GUIConfig;
    class Light;
    class PointLight;
    class AreaLight;
    class Box;
    class VirtualLight;
    class PhotonMap;
    class Random;
    class Task;
    class RenderTask;

    class BSDF;
    class Lambertian;

    class PostProcessor;
    class HDRPostProcessor;
    class BlurPostProcessor;
    class ScaleBlur;
    class ToneMapping;

    extern Engine gEngine;
    extern EngineConfig gEngineConfig;
    extern Random gRandom;


}



#endif // LYRAE_CORE_LYRAEFX_H