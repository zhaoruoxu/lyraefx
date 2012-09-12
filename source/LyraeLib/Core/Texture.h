#if defined(_MSC_VER)
#pragma once
#endif

#ifndef LYRAE_CORE_TEXTURE_H
#define LYRAE_CORE_TEXTURE_H

#include "LyraeFX.h"
#include "Geometry.h"

namespace LyraeFX
{
    typedef struct  
    {
        byte identsize;
        byte colormaptype;
        byte imagetype;

        short coloumapstart;
        short colormaplength;
        byte colormapbits;

        short xstart;
        short ystart;
        short width;
        short height;
        byte bits;
        byte descriptor;

    } TGA_HEADER;

    class Texture
    {
    public:

        Texture(const char *file);
        Texture(Color *bitmap, int width, int height)
            :mBitmap(bitmap), mWidth(width), mHeight(height) {}
        Texture(const Texture &t)
            :mWidth(t.mWidth), mHeight(t.mHeight)
        {
            int32_t size = mWidth * mHeight;
            mBitmap = new Color[size];
            memcpy(mBitmap, t.mBitmap, sizeof(Color) * size);
        }

        Texture &operator=(const Texture &rhs)
        {
            if ( mBitmap )
                delete [] mBitmap;
            int32_t size = rhs.mWidth * rhs.mHeight;
            mBitmap = new Color[size];
            memcpy(mBitmap, rhs.mBitmap, size);
            return *this;
        }
        virtual ~Texture()
        {
            if ( mBitmap ) delete [] mBitmap;
        }
        bool IsLoadSuccess() 
        {
            return mBitmap && mWidth > 0 && mHeight > 0 ;
        }

        Color GetTexel(float u, float v);

        Color *mBitmap;
        int mWidth;
        int mHeight;
        
    };
}

#endif // LYRAE_CORE_TEXTURE_H