#include "Material.h"

namespace LyraeFX
{
    Color Material::EvaluateE(const Vector &Vi, const Vector &N) const
    {
        Color r(0, 0, 0);
        for ( uint32_t i = 0; i < BSDFs.size(); i++ ) {
            r += BSDFs[i]->E(Vi, N);
        }
        return r;
    }

    Color Material::EvaluateF(const Vector &Vi, const Vector &Vo, const Vector &N) const
    {
        Color r(0, 0, 0);
        for ( uint32_t i = 0; i < BSDFs.size(); i++ ) {
            r += BSDFs[i]->F(Vi, Vo, N);
        }
        return r;
    }
}
