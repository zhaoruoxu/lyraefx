
#include "LyraeFX.h"
#include "LyraeAPI.h"
#include "Geometry.h"
#include "Parallel.h"
#include "Error.h"
#include "../Config/EngineConfig.h"

namespace LyraeFX
{
    void HDRToRGB(Pixel *dest, Color *src, int32_t width, int32_t height)
    {
        for ( int w = 0; w < width; w++ ) for ( int h = 0; h < height; h++ )
        {
            Color *pc = src + (h * width + w);
            int r = (int) (pc->x * 256);
            int g = (int) (pc->y * 256);
            int b = (int) (pc->z * 256);

            if ( r > 255 ) r = 255;
            if ( g > 255 ) g = 255;
            if ( b > 255 ) b = 255;

            dest[h * width + w] = (r << 16) + (g << 8) + b;
        }
    }

    bool LyraeFXRender()
    {
        bool verbose = gEngineConfig.GetBool("Verbose");
        gEngine.InitRender();
        gEngine.engineBusy = true;
        gEngine.engineOkay = true;
        gEngine.renderedPixels = 0;
        if ( verbose ) Info("Renderer initialized.");

        if ( verbose ) Info("Creating render threads...");
        int thdNum = gEngineConfig.GetInt32("RenderThreads");
        HANDLE *hThreads = new HANDLE[thdNum];
        RenderThreadParam *params = new RenderThreadParam[thdNum];
        int h = gEngineConfig.GetInt32("TargetHeight") / thdNum;

        DWORD time0 = timeGetTime();

        for ( int i = 0; i < thdNum; i++ ) {
            params[i].height1 = i * h;
            params[i].height2 = (i + 1) * h;
            if ( i == thdNum - 1 && params[i].height2 < gEngineConfig.GetInt32("TargetHeight") )
                params[i].height2 = gEngineConfig.GetInt32("TargetHeight");
            params[i].threadID = i;
        }

        if ( gEngineConfig.GetBool("EnablePhotonMapping") ) {
            for ( int i = 0; i < thdNum; i++ ) {
                hThreads[i] = (HANDLE) _beginthreadex(NULL, 0, PhotonThread, &params[i], 0, NULL);
            }
            WaitForMultipleObjects(thdNum, hThreads, TRUE, INFINITE);
        }

        if ( !gEngine.engineOkay ) goto RenderEnd;
        for ( int i = 0; i < thdNum; i++ ) {
            hThreads[i] = (HANDLE) _beginthreadex(NULL, 0, RenderWorkThread, &params[i], 0, NULL);
        }
        WaitForMultipleObjects(thdNum, hThreads, TRUE, INFINITE);

        if ( !gEngine.engineOkay ) goto RenderEnd;
        if ( gEngineConfig.GetBool("EnablePostProcessing") ) {
            if ( verbose ) Info("Post processing...");
            gEngine.PostProcess();
        }
        gEngine.Cleanup();

RenderEnd:
        delete [] hThreads;
        delete [] params;

        DWORD time1 = timeGetTime();
        stringstream ss;
        ss << "Render time: " << (time1 - time0) / 1000.f << " s";
        gEngine.renderTime = time1 - time0;
        Info(ss.str());
        gEngine.engineBusy = false;

        return gEngine.engineOkay;
    }
}

