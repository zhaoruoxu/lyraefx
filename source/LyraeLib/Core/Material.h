#if defined(_MSC_VER)
#pragma once
#endif

#ifndef LYRAE_CORE_MATERIAL_H
#define LYRAE_CORE_MATERIAL_H

#include "Geometry.h"
#include "Texture.h"
#include "Shading.h"

namespace LyraeFX
{

    class Material
    {
    public:
        Material() : mColor(1.f, 1.f, 1.f), mTexture(NULL), mUScale(1.f), mVScale(1.f) {}
        Material(const Color &c, Texture *tex = NULL, float u = 1.f, float v = 1.f)
            : mColor(c), mTexture(tex), mUScale(u), mVScale(v) {}
        virtual ~Material()
        {
            for ( uint32_t i = 0; i < BSDFs.size(); i++ ) {
                delete BSDFs[i];
            }
        }

        Color EvaluateE(const Vector &Vi, const Vector &N) const;
        Color EvaluateF(const Vector &Vi, const Vector &Vo, const Vector &N) const;
        inline bool MatchesBSDFFlags(BSDFType t);

        Color mColor;
        Texture *mTexture;
        float mUScale, mVScale;
        vector<BSDF *> BSDFs;
    }; // class Material

    inline bool Material::MatchesBSDFFlags(BSDFType t) 
    {
        for ( uint32_t i = 0; i < BSDFs.size(); i++ ) {
            if ( BSDFs[i]->MatchesFlags(t) ) return true;
        }
        return false;
    }
} // namespace LyraeFX

#endif // LYRAE_CORE_MATERIAL_H