#include "ToneMapping.h"

#include "../Core/LyraeFX.h"

namespace LyraeFX
{

    void ToneMapping::Process()
    {
        float aveLum = calculateAverageLuminace();
        float rAveLum = 1.f / aveLum;
        const int32_t total = mWidth * mHeight;
        for ( int32_t i = 0; i < total; i++ )
        {
            Color c = mRT->mBuffer[i];
            float pixelLum = calculateLuminace(&c);
            float scaledLum = 0.36f * pixelLum * rAveLum;

            float finalLum = scaledLum / ( 1 + scaledLum );
            c *= finalLum / pixelLum;

            c.Powf(0.7f);

            mRT->mBuffer[i] = c;
         
        }
    }

    float ToneMapping::calculateAverageLuminace()
    {
        int32_t total = mWidth * mHeight;
        float totalLum = 0.f;
        float totalr = 1.f / total;
        for ( int32_t i = 0; i < total; i++ )
        {
            float lum = calculateLuminace(mRT->mBuffer + i);
            totalLum += logf(lum + EPSILON);
        }
        return expf(totalLum * totalr);
    }
}