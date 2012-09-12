
#if defined(_MSC_VER)
#pragma once
#endif

#ifndef LYRAE_CORE_ENGINE_H
#define LYRAE_CORE_ENGINE_H

#include "LyraeFX.h"
#include "Scene.h"
#include "Geometry.h"
#include "Config.h"
#include "Light.h"
#include "Random.h"
#include "kdtree.h"
#include "PhotonMap.h"
#include "RenderTarget.h"
#include "LyraeAPI.h"
#include "Task.h"

#include "../Config/EngineConfig.h"

namespace LyraeFX
{

    class Engine
    {
    public:
        Engine();  
        virtual ~Engine();

        void            InitRender();
        bool            LoadSceneFromFile(string filename);
        bool            IsReady() { return mScene->IsInitialized(); }
        bool            IsBusy() { return engineBusy; }
        void            Terminate() { engineOkay = false; taskManager.Reset(); }
        Shape *         RayTrace(const Ray &ray, Color &acc, int depth, float dist, float samples);
        bool            Render(int32_t width0, int32_t width1, int32_t height1, int32_t height2);
        IntersectResult FindNearest(const Ray &ray, float &dist, Shape *&shape);
        Shape *         RenderRay(const Vector &screenPos, Color &acc);
        float           CalcShade(Light *light, const Vector &ip, Vector &dir, float samples);
        void            PreprocessVirtualLights();
        VirtualLight *  CreateVirtualLight(Light *l, const Vector &pos, const Vector &dir);
        Color           EvaluateVolumetricLighting(const Ray &ray, float dist);
        bool            PostProcess();
        float           EvaluateAmbientOcclusion(const Vector &pi, const Vector &N, int nSamples);

        inline void     Cleanup();
        inline void     AddInfo(string s);
        inline void     AddInfo(stringstream &ss);
        inline void     AddWarning(string s);
        inline void     AddWarning(stringstream &ss);
        inline void     AddSevere(string s);
        inline void     AddSevere(stringstream &ss);
        inline void     ClearInfo();
        inline void     ExtractInfo(vector<string> &v);
        inline void     ThreadBuildPhotonMap(uint32_t thdNum);
        inline void     ToRGBRenderTarget(RGBRenderTarget *r);

        Scene           *mScene;
        int             mWidth, mHeight, mPPos;
        Vector          mOrigin, mP1, mP2, mP3, mP4, mDy, mDz;
        Vector          mSR, mCW;
        Vector          mDOPDy, mDOPDz;
        int32_t         mDOPSamples, mDOPTotalSamples;
        Vector          mDOPDir;
        HDRRenderTarget *renderTarget;
        bool            engineOkay;
        bool            engineBusy;
        TaskManager     taskManager;
        DWORD           renderTime;
        PhotonMap       photonMap;
        AtomicInt32     renderedPixels;

    private:
        void LoadConfig();
        
        vector<string>          infos;
        vector<PostProcessor *> postProcessors;
        CRITICAL_SECTION        cs;
        AABB                    mVolumeRange;
        
        bool    confEnableAdaptiveSampling;
        bool    confEnableSSAA;
        bool    confEnableSoftShadow;
        bool    confEnableShadow;
        bool    confEnablePostProcessing;
        bool    confEnableIGI;
        bool    confEnableVolumetricLighting;

        bool    confVerbose;
        int     confCameraType;
        Vector  confTargetPosition;
        int     confTraceDepth;
        int     confRenderThreads;
        int     confSSAASamples;
        int     confSoftShadowSamples;
        int     confGlossyReflectionSamples;
        int     confIGISamples;

        float   confImpSamples;
        float   confImpScale;

        float   confIGIVirLightDist;
        float   confIGIVirLightIntensity;
        float   confIGIVirLightAlpha;

        Vector  confVolumetricLightingP1;
        Vector  confVolumetricLightingP2;
        float   confVolumetricLightingEnhancer;
        float   confVolumetricLightingMaximumDistance;
        float   confVolumetricLightingSampleStep;

        bool    confEnableAmbientOcclusion;
        Color   confAmbientOcclusionColor;
        float   confAmbientOcclusionDistance;
        float   confAmbientOcclusionEnhancer;
        int     confAmbientOcclusionSamples;
    }; // class Engine

    inline void Engine::ToRGBRenderTarget(RGBRenderTarget *r)
    {
        if ( r == NULL || renderTarget == NULL ) return;
        HDRToRGB(r->mBuffer, renderTarget->mBuffer, 
            gEngineConfig.GetInt32("TargetWidth"), 
            gEngineConfig.GetInt32("TargetHeight"));
    }

    inline void Engine::Cleanup()
    {
        photonMap.CleanupPhotons();
        mWidth = mHeight = 0;
    }

    inline void Engine::AddInfo(string s)
    {
        EnterCriticalSection(&cs);
        char bufTime[128], bufDate[128];
        _strdate_s(bufDate);
        _strtime_s(bufTime);
        infos.push_back("[" + string(bufDate) + " " + string(bufTime) + "] " + s);
        LeaveCriticalSection(&cs);
    }
    inline void Engine::AddInfo(stringstream &ss)
    {
        AddInfo(ss.str());
        ss.str("");
        ss.clear();
    }
    inline void Engine::AddWarning(string s)
    {
        EnterCriticalSection(&cs);
        char bufTime[128], bufDate[128];
        _strdate_s(bufDate);
        _strtime_s(bufTime);
        infos.push_back("[" + string(bufDate) + " " + string(bufTime) + "] WARNING: " + s);
        LeaveCriticalSection(&cs);
    }
    inline void Engine::AddWarning(stringstream &ss)
    {
        AddWarning(ss.str());
        ss.str("");
        ss.clear();
    }

    inline void Engine::AddSevere(string s)
    {
        EnterCriticalSection(&cs);
        engineOkay = false;
        taskManager.Reset();
        char bufTime[128], bufDate[128];
        _strdate_s(bufDate);
        _strtime_s(bufTime);
        infos.push_back("[" + string(bufDate) + " " + string(bufTime) + "] SEVERE: " + s);
        LeaveCriticalSection(&cs);
    }
    inline void Engine::AddSevere(stringstream &ss)
    {
        AddSevere(ss.str());
        ss.str("");
        ss.clear();
    }

    inline void Engine::ClearInfo()
    {
        EnterCriticalSection(&cs);
        infos.clear();
        LeaveCriticalSection(&cs);
    }
    inline void Engine::ExtractInfo(vector<string> &v)
    {
        EnterCriticalSection(&cs);
        v.reserve(infos.size());
        for ( uint32_t i = 0; i < infos.size(); i++ ) {
            v.push_back(infos[i]);
        }
        LeaveCriticalSection(&cs);
    }

    inline void Engine::ThreadBuildPhotonMap(uint32_t thdNum)
    {
        photonMap.ThreadBuildPhotonMap(thdNum);
    }

    extern Engine gEngine;

} // namespace LyraeFX


#endif // LYRAE_CORE_ENGINE_H