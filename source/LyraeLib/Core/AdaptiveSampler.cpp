#include "AdaptiveSampler.h"
#include "Random.h"
#include "Geometry.h"

namespace LyraeFX
{
    void RequestSamples(float *dest, int32_t dim)
    {
        float d = 1.f / dim;
        float x0 = -0.5f + d * 0.5f;
        float y0 = -0.5f + d * 0.5f;
        uint32_t pos = 0;
        float x = x0;
        for ( int i = 0; i < dim; i++ ) {
            float y = y0;
            for ( int j = 0; j < dim; j++ ) {
                dest[pos++] = x;
                dest[pos++] = y;
                y += d;
            }
            x += d;
        }
    }
    bool NeedSupersampling(Color *c, int32_t n, float threshold)
    {
        float lavg = 0.f;
        for ( int i = 0; i < n; i++ ) {
            lavg += c[i].Luminance();
        }
        lavg /= n;
        for ( int i = 0; i < n; i++ ) {
            if ( fabsf(c[i].Luminance() - lavg) / lavg > threshold ) {
                return true;
            }
        }
        return false;
    }
}