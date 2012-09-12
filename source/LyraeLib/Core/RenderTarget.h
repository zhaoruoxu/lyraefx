#if defined(_MSC_VER)
#pragma once
#endif

#ifndef LYRAE_CORE_RENDERTARGET_H
#define LYRAE_CORE_RENDERTARGET_H

#include "LyraeFX.h"
#include "Geometry.h"

namespace LyraeFX
{
    class RGBRenderTarget
    {
    public:
        RGBRenderTarget(int32_t width, int32_t height)
            :mWidth(width), mHeight(height)
        {
            mBuffer = new Pixel[width * height];
        }
        virtual ~RGBRenderTarget()
        {
            delete [] mBuffer;
        }
        void Clear(Pixel color)
        {
            for ( int32_t i = 0; i < mWidth * mHeight; i++ )
            {
                mBuffer[i] = color;
            }
        }
        Pixel *mBuffer;
        int32_t mWidth;
        int32_t mHeight;

    }; // class RenderTarget

    class HDRRenderTarget
    {
    public:
        HDRRenderTarget(int32_t width, int32_t height)
            :mWidth(width), mHeight(height)
        {
            mBuffer = new Color[width * height];
        }
        virtual ~HDRRenderTarget()
        {
            delete [] mBuffer;
        }

        void SetPixel(int32_t pos, const Color &color)
        {
            _ASSERT(pos >= 0 && pos <= mWidth * mHeight);
            mBuffer[pos] = color;
        }

        void Clear(Color color)
        {
            for ( int32_t i = 0; i < mWidth * mHeight; i++ )
            {
                mBuffer[i] = color;
            }
        }
        bool CopyFrom(const HDRRenderTarget &t)
        {
            if ( mWidth != t.mWidth || mHeight != t.mHeight )
                return false;

            memcpy(mBuffer, t.mBuffer, mWidth * mHeight * sizeof(Color));
            return true;
        }

        bool Merge(const HDRRenderTarget &t, float w1, float w2)
        {
            if ( mWidth != t.mWidth || mHeight != t.mHeight )
                return false;
            for ( int i = 0; i < mWidth * mHeight; i++ )
            {
                mBuffer[i] = mBuffer[i] * w1 + t.mBuffer[i] * w2;
            }
            return true;
        }


        Color *mBuffer;
        int32_t mWidth;
        int32_t mHeight;
    };

} // namespace LyraeFX

#endif // LYRAE_CORE_RENDERTARGET_H