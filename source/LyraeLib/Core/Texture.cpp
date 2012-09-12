#include "Texture.h"
#include "../Config/EngineConfig.h"

namespace LyraeFX
{
    Texture::Texture(const char *file)
        :mBitmap(NULL), mWidth(0), mHeight(0)
    {
        ILuint texID;
        ilGenImages(1, &texID);
        ilBindImage(texID);
        ILboolean success = ilLoadImage(file);
        if ( !success ) return;

        mWidth = ilGetInteger(IL_IMAGE_WIDTH);
        mHeight = ilGetInteger(IL_IMAGE_HEIGHT);
        int bpp = ilGetInteger(IL_IMAGE_BPP);

        ILubyte *data = ilGetData();
        mBitmap = new Color[mWidth * mHeight];
        const float r = 1.f / 256;
        if ( bpp == 12 )
        {
            for ( int size = mWidth * mHeight, i = 0; i < size; i++ )
            {
                float b = *((float *) (data + i * 12 + 8));
                float g = *((float *) (data + i * 12 + 4));
                float r = *((float *) (data + i * 12));
                mBitmap[i] = Color(r, g, b);
            }
        }
        else
        {
            for ( int size = mWidth * mHeight, i = 0; i < size; i++ )
            {
                mBitmap[i] = Color(data[i * bpp + 0] * r, data[i * bpp + 1] * r, data[i * bpp + 2] * r);
            }
        }

        ilDeleteImage(texID);
    }

    Color Texture::GetTexel(float u, float v)
    {
        _ASSERT(u >= 0.f);
        _ASSERT(v >= 0.f);
        if ( u >= 1.f ) u -= floorf(u);
        if ( v >= 1.f ) v -= floorf(v);
        float fu = u * mWidth;
        float fv = v * mHeight;
        float fu0 = floorf(fu);
        float fv0 = floorf(fv);
        int32_t iu = ((int32_t) fu0) % mWidth;
        int32_t iv = ((int32_t) fv0) % mHeight;
        float uratio = fu - fu0;
        float vratio = fv - fv0;
        float uopposite = 1 - uratio;
        float vopposite = 1 - vratio;
        Color c0 = mBitmap[iu + iv * mWidth] * uopposite * vopposite;
        Color c1 = mBitmap[(iu + 1) % mWidth + iv * mWidth] * uratio * vopposite;
        Color c2 = mBitmap[iu + ((iv + 1) % mHeight) * mWidth] * uopposite * vratio;
        Color c3 = mBitmap[(iu + 1) % mWidth + ((iv + 1) % mHeight) * mWidth] * uratio * vratio;
        return c0 + c1 + c2 + c3;
        
    }
}