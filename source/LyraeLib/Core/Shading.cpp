#include "Shading.h"

namespace LyraeFX
{
    Color Lambertian::F(const Vector &Vi, const Vector &Vo, const Vector &N) const
    {
        float cosA = N.Dot(Vo);
        if ( cosA < 0.f ) return Color(0, 0, 0);
        return color * cosA;
    }

    Color Phong::E(const Vector &Vi, const Vector &N) const 
    {
        return ambientColor * ambient;
    }

    Color Phong::F(const Vector &Vi, const Vector &Vo, const Vector &N) const
    {
        Color c(0, 0, 0);
        float cosA = N.Dot(Vo);
        if ( cosA > 0.f ) c += diffuseColor * (cosA * diffuse);
        Vector r = 2.0f * Vo.Dot(N) * N - Vo;
        float cosS = Vi.Dot(r);
        if ( cosS > 0.f ) {
            float spec = powf(cosS, exp) * specular;
            c += specularColor * spec;
        }
        return c;
    }

    float FresnelDielectric::Evaluate(float cosi) const
    {
        Clamp(cosi, -1.f, 1.f);
        bool entering = cosi > 0;
        float ei = eta_i, et = eta_t;
        if ( !entering ) swap(ei, et);
        float sint = ei / et * sqrtf(max(0.f, 1.f - cosi * cosi));
        if ( sint >= 1.f ) {
            return 1.f;
        } else {
            float cost = sqrtf(max(0.f, 1.f - sint * sint));
            cosi = fabsf(cosi);

            float rparl = (eta_t * cosi - eta_i * cost) /
                          (eta_t * cosi + eta_i * cost);
            float rperp = (eta_i * cosi - eta_t * cost) / 
                          (eta_i * cosi + eta_t * cost);
            return (rparl * rparl + rperp * rperp) * 0.5f;
        }
    }

    float FresnelConductor::Evaluate(float cosi) const
    {
        cosi = fabsf(cosi);
        float tmp = (eta * eta + k * k) * cosi * cosi;
        float rparl2 = (tmp - 2.f * eta * cosi + 1) /
                       (tmp + 2.f * eta * cosi + 1);
        float tmpf = eta * eta + k * k;
        float rperp2 = (tmpf - 2.f * eta * cosi + cosi * cosi) /
                       (tmpf + 2.f * eta * cosi + cosi * cosi);
        return 0.5f * (rparl2 + rperp2);
    }

    bool SpecularReflection::G(const Vector &Vi, Vector *&Vos, int &voNum, const Vector &N, Color &c, float scale)
    {
        float cosi = Vi.Dot(N);
        Vos = new Vector[1];
        voNum = 1;
        Vos[0] = 2.f * cosi * N - Vi;
        float coso = Vos[0].Dot(N);
        c = fresnel->Evaluate(cosi) * color;
        return true;
    }

    bool GlossyReflection::G(const Vector &Vi, Vector *&Vos, int &voNum, const Vector &N, Color &c, float scale)
    {
        float cosi = Vi.Dot(N);
        voNum = static_cast<int>(nSamples * scale);
        voNum = max(voNum, 1);
        Vos = new Vector[voNum];
        Vector Vo = 2.f * cosi * N - Vi;
        Vo.Normalize();
        Vector RN1(Vo.z, Vo.y, -Vo.x);
        Vector RN2 = Vo.Cross(RN1);
        for ( int i = 0; i < voNum; i++ ) {
            while ( true ) {
                Vos[i] = Vo + RN1 * (gRandom.Rand() * glossyScale * scale) 
                            + RN2 * (gRandom.Rand() * glossyScale * scale);
                if ( Vos[i].Dot(N) > 0 ) break;
            }
            Vos[i].Normalize();
        }
        c = fresnel->Evaluate(cosi) * color;
        return true;
    }

    bool SpecularTransmission::G(const Vector &Vi, Vector *&Vos, int &voNum, const Vector &N, Color &c, float scale)
    {
        Vector norm = N;
        float cosi = Vi.Dot(norm);
        Clamp(cosi, -1.f, 1.f);
        float ei = eta_i, et = eta_t;
        if ( cosi < 0 ) {
            norm = -N;
            cosi = -cosi;
            swap(ei, et);
        }
        float n = ei / et;
        float sini2 = 1 - cosi * cosi;
        float sint2= n * n * sini2;
        if ( sint2 > 1.f ) {
            return false;
        }
        float cost = sqrtf(max(0.f, 1.f - sint2));
        float tant;
        if ( cost == 0 ) tant = 1e8f;
        else tant = sqrtf(sint2) / cost;
        float tani;
        if ( cosi == 0 ) tani = 1e8f;
        else tani = sqrtf(sini2) / cosi;
        float xratio = tani == 0 ? 0 : tant / tani;
        Vector X = (norm - Vi) * xratio;
        Vos = new Vector[1];
        voNum = 1;
        Vos[0] = (X - norm).GetNormalizedVector();
        c = (1.f - fresnel.Evaluate(cosi)) * color;
        return true;
    }

    bool GlossyTransmission::G(const Vector &Vi, Vector *&Vos, int &voNum, const Vector &N, Color &c, float scale)
    {
        Vector norm = N;
        float cosi = Vi.Dot(norm);
        Clamp(cosi, -1.f, 1.f);
        float ei = eta_i, et = eta_t;
        if ( cosi < 0 ) {
            norm = -N;
            cosi = -cosi;
            swap(ei, et);
        }
        float n = ei / et;
        float sini2 = 1 - cosi * cosi;
        float sint2= n * n * sini2;
        if ( sint2 > 1.f ) {
            return false;
        }
        float cost = sqrtf(max(0.f, 1.f - sint2));
        float tant;
        if ( cost == 0 ) tant = 1e8f;
        else tant = sqrtf(sint2) / cost;
        float tani;
        if ( cosi == 0 ) tani = 1e8f;
        else tani = sqrtf(sini2) / cosi;
        float xratio = tani == 0 ? 0 : tant / tani;
        Vector X = (norm - Vi) * xratio;
        Vector Vo = (X - norm).GetNormalizedVector();

        voNum = static_cast<int>(nSamples * scale);
        voNum = max(voNum, 1);
        Vos = new Vector[voNum];
        Vector RN1(Vo.z, Vo.y, -Vo.x);
        Vector RN2 = Vo.Cross(RN1);
        for ( int i = 0; i < voNum; i++ ) {
            while ( true ) {
                Vos[i] = Vo + RN1 * (gRandom.Rand() * glossyScale * scale) 
                            + RN2 * (gRandom.Rand() * glossyScale * scale);
                if ( Vos[i].Dot(norm) < 0 ) break;
            }
            Vos[i].Normalize();
        }

        c = (1.f - fresnel.Evaluate(cosi)) * color;
        return true;
    }

    Color SimpleLight::E(const Vector &Vi, const Vector &N) const
    {
        Color c(0, 0, 0);
        float cosTheta = Vi.Dot(N);
        if ( cosTheta < 0 ) return c;
        c = mColor * mIntensity * cosTheta * mAlpha;
        Clamp(c, Color(0, 0, 0), mColor * mIntensity);
        return c;
    }

    Color SimpleLight::F(const Vector &Vi, const Vector &Vo, const Vector &N) const
    {
        return Color(0, 0, 0);
    }
}