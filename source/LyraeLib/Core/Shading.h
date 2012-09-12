#if defined(_MSC_VER)
#pragma once
#endif

#ifndef LYRAE_CORE_SHADING_H
#define LYRAE_CORE_SHADING_H

#include "LyraeFX.h"
#include "Geometry.h"
#include "Light.h"

namespace LyraeFX
{
    enum BSDFType
    {
        BSDF_REFLECTION     = 1 << 0,
        BSDF_TRANSMISSION   = 1 << 1,
        BSDF_DIFFUSE        = 1 << 2,
        BSDF_SPECULAR       = 1 << 3,
        BSDF_GLOSSY         = 1 << 4
    };
    class BSDF
    {
    public:
        BSDF(BSDFType t) : type(t) {}
        virtual ~BSDF() {}
        bool MatchesFlags(BSDFType t) const {
            return ( type & t ) == t;
        }
        virtual Color E(const Vector &Vi, const Vector &N) const
        {
            return Color(0, 0, 0);
        }
        virtual Color F(const Vector &Vi, const Vector &Vo, const Vector &N) const = 0;
        virtual bool G(const Vector &Vi, Vector *&Vos, int &voNum, const Vector &N, Color &color, float scale) = 0;

        const BSDFType type;
    };

    class Lambertian : public BSDF
    {
    public:
        Lambertian(const Color &r) : BSDF(BSDFType(BSDF_REFLECTION | BSDF_DIFFUSE)), color(r) {}
        ~Lambertian() {};
        Color F(const Vector &Vi, const Vector &Vo, const Vector &N) const;
        bool G(const Vector &Vi, Vector *&Vos, int &voNum, const Vector &N, Color &color, float scale)
        {
            return false;
        }
    private:
        Color color;
    };

    class Phong : public BSDF
    {
    public:
        Phong(const Color &dc, const Color &sc, const Color &ac, float d, float s, float a, float e)
            : BSDF(BSDFType(BSDF_REFLECTION | BSDF_DIFFUSE)), diffuseColor(dc), specularColor(sc), 
              ambientColor(ac), diffuse(d), specular(s), ambient(a), exp(e) {}
        ~Phong() {}
        Color E(const Vector &Vi, const Vector &N) const;
        Color F(const Vector &Vi, const Vector &Vo, const Vector &N) const;
        bool G(const Vector &Vi, Vector *&Vos, int &voNum, const Vector &N, Color &color, float scale) { return false; }

    private:
        Color diffuseColor;
        Color specularColor;
        Color ambientColor;
        float diffuse;
        float specular;
        float ambient;
        float exp;
    };

    class Fresnel
    {
    public:
        virtual ~Fresnel() {}
        virtual float Evaluate(float cosi) const = 0;
    };

    class FresnelDielectric : public Fresnel
    {
    public:
        FresnelDielectric(float ei, float et) : eta_i(ei), eta_t(et) {}
        float Evaluate(float cosi) const;
    private:
        float eta_i, eta_t;
    };

    class FresnelConductor : public Fresnel
    {
    public:
        FresnelConductor(float e, float _k) : eta(e), k(_k) {}
        float Evaluate(float cosi) const;
    private:
        float eta, k;
    };

    class FresnelSpecial : public Fresnel
    {
    public:
        FresnelSpecial(float s) : scale(s) {}
        float Evaluate(float cosi) const
        {
            return scale;
        }

    private:
        float scale;
    };

    class SpecularReflection : public BSDF
    {
    public:
        SpecularReflection(const Color &r, Fresnel *f)
            : BSDF(BSDFType(BSDF_REFLECTION | BSDF_SPECULAR)), color(r), fresnel(f) {}
        virtual ~SpecularReflection()
        {
            delete fresnel;
        }
        Color F(const Vector &Vi, const Vector &Vo, const Vector &N) const
        {
            return Color(0, 0, 0);
        }
        bool G(const Vector &Vi, Vector *&Vos, int &voNum, const Vector &N, Color &color, float scale);

    private:
        Color color;
        Fresnel *fresnel;
    };

    class SpecularTransmission : public BSDF
    {
    public:
        SpecularTransmission(const Color &c, float ei, float et)
            : BSDF(BSDFType(BSDF_TRANSMISSION | BSDF_SPECULAR)), color(c), fresnel(ei, et), eta_i(ei), eta_t(et)
        {
        }
        Color F(const Vector &Vi, const Vector &Vo, const Vector &N) const
        {
            return Color(0, 0, 0);
        }
        bool G(const Vector &Vi, Vector *&Vos, int &voNum, const Vector &N, Color &color, float scale);

    private:
        Color color;
        float eta_i, eta_t;
        FresnelDielectric fresnel;
    };

    class GlossyTransmission : public BSDF
    {
    public:
        GlossyTransmission(const Color &c, float ei, float et, float gs, int32_t n)
            : BSDF(BSDFType(BSDF_TRANSMISSION | BSDF_GLOSSY)), color(c), fresnel(ei, et), eta_i(ei), eta_t(et),
              glossyScale(gs), nSamples(n) {}

        Color F(const Vector &Vi, const Vector &Vo, const Vector &N) const
        {
            return Color(0, 0, 0);
        }
        bool G(const Vector &Vi, Vector *&Vos, int &voNum, const Vector &N, Color &color, float scale);

    private:
        Color color;
        float eta_i, eta_t;
        FresnelDielectric fresnel;
        float glossyScale;
        int32_t nSamples;
    };

    class GlossyReflection : public BSDF
    {
    public:
        GlossyReflection(const Color &r, Fresnel *f, float gs, int32_t n)
            : BSDF(BSDFType(BSDF_REFLECTION | BSDF_GLOSSY)), color(r), fresnel(f),
              glossyScale(gs), nSamples(n) {}
        virtual ~GlossyReflection()
        {
            delete fresnel;
        }
        Color F(const Vector &Vi, const Vector &Vo, const Vector &N) const
        {
            return Color(0, 0, 0);
        }
        bool G(const Vector &Vi, Vector *&Vos, int &voNum, const Vector &N, Color &color, float scale);

    private:
        Color color;
        Fresnel *fresnel;
        float glossyScale;
        int32_t nSamples;
    };

    class SimpleLight : public BSDF
    {
    public:
        SimpleLight(const Color &c, float i, float a) :BSDF(BSDFType(BSDF_DIFFUSE | BSDF_REFLECTION)),
            mColor(c), mIntensity(i), mAlpha(a) {}
        ~SimpleLight() {}

        Color E(const Vector &Vi, const Vector &N) const;
        Color F(const Vector &Vi, const Vector &Vo, const Vector &N) const;
        bool G(const Vector &Vi, Vector *&Vos, int &voNum, const Vector &N, Color &color, float scale) { return false; }

        float mAlpha;
        Color mColor;
        float mIntensity;
    };
}


#endif // LYRAE_CORE_SHADING_H