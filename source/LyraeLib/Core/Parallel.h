#if defined(_MSC_VER)
#pragma once
#endif

#ifndef LYRAE_CORE_PARALLEL_H
#define LYRAE_CORE_PARALLEL_H

#include "LyraeFX.h"

namespace LyraeFX
{
    extern "C" void _ReadWriteBarrier();
#pragma intrinsic(_ReadWriteBarrier)

    inline int32_t AtomicAdd(AtomicInt32 *v, int32_t delta) {
        int32_t result;
        _ReadWriteBarrier();
        __asm {
            __asm mov edx, v
            __asm mov eax, delta
            __asm lock xadd [edx], eax
            __asm mov result, eax
        }
        _ReadWriteBarrier();
        return result + delta;
    }

    struct RenderThreadParam
    {
        int height1;
        int height2;
        int threadID;

        RenderThreadParam()
            :height1(0), height2(2), threadID(0) {}
        RenderThreadParam(int h1, int h2, LyraeFX::Engine *t, int id)
            :height1(h1), height2(h2), threadID(id) {}
    };

    unsigned int __stdcall RenderThread(LPVOID pParam);
    unsigned int __stdcall RenderWorkThread(LPVOID param);
    unsigned int __stdcall PhotonThread(LPVOID param);
}


#endif // LYRAE_CORE_PARALLEL_H