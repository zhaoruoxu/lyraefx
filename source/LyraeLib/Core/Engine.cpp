#include "Engine.h"
#include "Scene.h"
#include "../Config/EngineConfig.h"
#include "../Light/PointLight.h"
#include "../Shape/Box.h"
#include "../Light/AreaLight.h"
#include "Random.h"
#include "../Shape/Plane.h"
#include "LyraeFX.h"
#include "../Light/AreaLight.h"
#include "../Light/PointLight.h"
#include "../Light/VirtualLight.h"
#include "PhotonMap.h"
#include "Parallel.h"
#include "Error.h"
#include "AdaptiveSampler.h"
#include "PostProcessor.h"
#include "../PostProcessor/HDRPostProcessor.h"

namespace LyraeFX
{
    Engine::Engine()
        :mScene(new Scene()), photonMap(mScene, &cs),
        confVolumetricLightingP1(0, 0, 0),
        confVolumetricLightingP2(0, 0, 0)
    {
        engineOkay = true;
        engineBusy = false;
        ilInit();
        InitializeCriticalSection(&cs);
        _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
    }

    Engine::~Engine() 
    {
        DeleteCriticalSection(&cs);
        photonMap.CleanupPhotons();
        delete mScene;
        mScene = NULL;
        if ( renderTarget ) {
            delete renderTarget;
            renderTarget = NULL;
        }
    }

    void Engine::LoadConfig()
    {
        mScene->LoadConfig();
        photonMap.LoadConfig();

        confEnableAdaptiveSampling      = gEngineConfig.GetBool("EnableAdaptiveSampling");
        confEnableSSAA                  = gEngineConfig.GetBool("EnableSSAA");
        confEnableSoftShadow            = gEngineConfig.GetBool("EnableSoftShadow");
        confEnableShadow                = gEngineConfig.GetBool("EnableShadow");
        confEnablePostProcessing        = gEngineConfig.GetBool("EnablePostProcessing");
        confEnableIGI                   = gEngineConfig.GetBool("EnableIGI");
        confEnableVolumetricLighting    = gEngineConfig.GetBool("EnableVolumetricLighting");

        confVerbose                     = gEngineConfig.GetBool("Verbose");
        confCameraType                  = gEngineConfig.GetInt32("CameraType");
        confTargetPosition              = gEngineConfig.GetVector("TargetPosition");
        confTraceDepth                  = gEngineConfig.GetInt32("TraceDepth");
        confRenderThreads               = gEngineConfig.GetInt32("RenderThreads");

        confSSAASamples                 = gEngineConfig.GetInt32("SSAASamples");
        confSoftShadowSamples           = gEngineConfig.GetInt32("SoftShadowSamples");
        confGlossyReflectionSamples     = gEngineConfig.GetInt32("GlossyReflectionSamples");
        confIGISamples                  = gEngineConfig.GetInt32("IGISamples");
        confImpSamples                  = gEngineConfig.GetFloat("ImpSamples");
        confImpScale                    = gEngineConfig.GetFloat("ImpScale");

        confIGIVirLightDist             = gEngineConfig.GetFloat("IGIVirLightDist");
        confIGIVirLightIntensity        = gEngineConfig.GetFloat("IGIVirLightIntensity");
        confIGIVirLightAlpha            = gEngineConfig.GetFloat("IGIVirLightAlpha");

        confVolumetricLightingEnhancer  = gEngineConfig.GetFloat("VolumetricLightingEnhancer");
        confVolumetricLightingMaximumDistance   = gEngineConfig.GetFloat("VolumetricLightingMaximumDistance");
        confVolumetricLightingSampleStep    = gEngineConfig.GetFloat("VolumetricLightingSampleStep");
        confVolumetricLightingP1            = gEngineConfig.GetVector("VolumetricLightingP1");
        confVolumetricLightingP2            = gEngineConfig.GetVector("VolumetricLightingP2");

        confEnableAmbientOcclusion      = gEngineConfig.GetBool("EnableAmbientOcclusion");
        confAmbientOcclusionColor       = gEngineConfig.GetVector("AmbientOcclusionColor");
        confAmbientOcclusionDistance    = gEngineConfig.GetFloat("AmbientOcclusionDistance");
        confAmbientOcclusionEnhancer    = gEngineConfig.GetFloat("AmbientOcclusionEnhancer");
        confAmbientOcclusionSamples     = gEngineConfig.GetInt32("AmbientOcclusionSamples");
        
        mVolumeRange.mPos = confVolumetricLightingP1;
        mVolumeRange.mSize = confVolumetricLightingP2 - confVolumetricLightingP1;
    }

    bool Engine::LoadSceneFromFile(string filename)
    {
        bool engineBusyStatus = engineBusy;
        engineBusy = true;
        bool loadResult = mScene->InitScene(filename);
        engineBusy = engineBusyStatus;
        engineOkay = loadResult;
        return loadResult;
    }

    void Engine::InitRender()
    {
        // set common flag
        engineOkay = true;
        engineBusy = false;

        //mScene->InitScene();
        if ( !mScene->IsInitialized() ) {
            Severe("No scene loaded.");
            return;
        }
        LoadConfig();

        // Init render target
        if ( confVerbose ) Info("Initializing render target...");
        int32_t targetWidth = gEngineConfig.GetInt32("TargetWidth");
        int32_t targetHeight = gEngineConfig.GetInt32("TargetHeight");
        if ( renderTarget ) { delete renderTarget; renderTarget = NULL; }
        renderTarget = new HDRRenderTarget(targetWidth, targetHeight);
        mWidth = targetWidth;
        mHeight = targetHeight;

        // Init camera and DOP
        if ( confVerbose ) Info("Initializing camera...");
        float wy = gEngineConfig.GetFloat("ProjPlaneHalfWidth"); 
        float wz = gEngineConfig.GetFloat("ProjPlaneHalfHeight"); 
        mDOPDir = Vector(-1, 0, 0);
        mOrigin = Vector(gEngineConfig.GetFloat("EyePosX"), 0.f, 0.f);
        
        mP1 = Vector(0, wy, wz);
        mP2 = Vector(0, -wy, wz);
        mP3 = Vector(0, -wy, -wz);
        mP4 = Vector(0, wy, -wz);
        Vector pos = gEngineConfig.GetVector("CameraPosition");

        float dopratio = pos.Length() / mOrigin.Length();
        Vector xAxis = confTargetPosition - pos;
        xAxis.Normalize();
        const Vector up = Vector(0, 0, 1);
        Vector yAxis = up.Cross( xAxis );
        Vector zAxis = xAxis.Cross( yAxis );
        yAxis.Normalize();
        zAxis.Normalize();
        Matrix m;
        m.a11 = xAxis.x; m.a12 = xAxis.y; m.a13 = xAxis.z;
        m.a21 = yAxis.x; m.a22 = yAxis.y; m.a23 = yAxis.z;
        m.a31 = zAxis.x; m.a32 = zAxis.y; m.a33 = zAxis.z;
        m.Invert();
        m.a14 = pos.x; m.a24 = pos.y; m.a34 = pos.z;
        
        mOrigin = m.Transform(mOrigin);
        mP1 = m.Transform(mP1);
        mP2 = m.Transform(mP2);
        mP3 = m.Transform(mP3);
        mP4 = m.Transform(mP4);
        
        mDy = (mP2 - mP1) * (1.f / mWidth);
        mDz = (mP4 - mP1) * (1.f / mHeight);
        int32_t samples = mScene->mDOPSamples;
        int32_t total = (samples * 2 + 1) * (samples * 2 + 1);
        m.a14 = m.a24 = m.a34 = 0.f;
        mDOPDir = m.Transform(mDOPDir);
        float rs = (samples == 0 ? 0.f : 1.f / samples) * dopratio;
        mDOPDy = mScene->mDOPRange * 2.f * (mP2 - mP1) * rs / (float) mWidth;
        mDOPDz = mScene->mDOPRange * 2.f * (mP4 - mP1) * rs / (float) mHeight;
        mDOPSamples = samples;
        mDOPTotalSamples = total;

        // IGI
        if ( confEnableIGI && !gEngineConfig.GetBool("EnablePhotonMapping")) {
            PreprocessVirtualLights();
        }
        // Photon mapping
        photonMap.InitPhotonMap();

        taskManager.Reset();
        taskManager.InitTasks();
    }

    bool Engine::PostProcess()
    {
        _ASSERT(renderTarget);
        // create post processors
        postProcessors.push_back(new HDRPostProcessor(renderTarget));
        // process
        for ( uint32_t i = 0; i < postProcessors.size(); i++ ) {
            postProcessors[i]->Process();
        }
        // delete post processors
        for ( uint32_t i = 0; i < postProcessors.size(); i++ ) {
            delete postProcessors[i];
        }
        postProcessors.clear();
        return true;
    }

    void Engine::PreprocessVirtualLights()
    {
#define CREATE_VLIGHT(x, y, z)  vl = CreateVirtualLight(light, pos, Vector((x), (y), (z))); \
                        if ( vl ) vlights.push_back(vl);

        vector<Light *> vlights;
        for ( LightIter iter = mScene->mLights.begin(); iter != mScene->mLights.end(); iter++ ) {
            Light *light = *iter;
            Vector pos = light->GetCentroid();
            int32_t samples = confIGISamples;
            _ASSERT(samples > 0);
            float d = 2.f / samples;
            
            for ( int32_t i = 0; i < samples; i++ ) {
                for ( int32_t j = 0; j < samples; j++ ) {
                    float fi = -1.f + i * d + d * gRandom.Rand();
                    float fj = -1.f + j * d + d * gRandom.Rand();
                    VirtualLight *vl;
                    CREATE_VLIGHT(fi, fj, -1); CREATE_VLIGHT(fi, fj, 1);
                    CREATE_VLIGHT(fi, -1, fj); CREATE_VLIGHT(fi, 1, fj);
                    CREATE_VLIGHT(-1, fi, fj); CREATE_VLIGHT(1, fi, fj);
                }
            }
        }

        for ( LightIter iter = vlights.begin(); iter != vlights.end(); iter++ ) {
            mScene->mLights.push_back(*iter);
        }
        vlights.clear();
#undef CREATE_VLIGHT
    }

    VirtualLight *Engine::CreateVirtualLight(Light *l, const Vector &pos, const Vector &adir)
    {
        Vector dir = adir.GetNormalizedVector();
        Shape *shape;
        float dist = MAXDISTANCE;
        Ray r(pos, dir);
        IntersectResult iresult = FindNearest(r, dist, shape);
        if ( iresult == NULL ) return NULL;
        Vector pi = pos + dir * dist;
        Vector N = shape->GetNormal(pi) * (float) iresult;
        Vector L = pos - pi;
        
        float dot = N.Dot(L);
        if ( dot <= 0.f ) return NULL;
        Vector vpi = pi + N * dist * confIGIVirLightDist;
        Color sColor = shape->GetColor(pi);
        Color lColor = l->Sample(pi);
        float a = dist / (dist + confIGIVirLightAlpha);
        float i = a * l->mIntensity * dot * confIGIVirLightIntensity;
        Color vColor = sColor * lColor;
        return new VirtualLight(vpi, vColor, i, shape);
    }

    IntersectResult Engine::FindNearest(const Ray &ray, float &dist, Shape *&shape)
    {
        return mScene->bvhAccel->FindNearest(ray, dist, shape);
    }

    Shape *Engine::RenderRay(const Vector &screenPos, Color &acc)
    {
        Vector dir;
        Ray r;
        if ( confCameraType == CAMERA_PERSPECTIVE ) {
            dir = screenPos - mOrigin;
            dir.Normalize();
            r = Ray(mOrigin, dir);
        } else if ( confCameraType == CAMERA_ORTHOGRAPHIC ) {
            dir = mOrigin - confTargetPosition;
            dir.Normalize();
            r = Ray(screenPos, -dir);
        } else {
            Warning("Unknown camera type");
            _ASSERT(0);
        }

        if ( mScene->mDOPEnabled ) {
            Plane fp(NULL, mDOPDir, mScene->mDOPFocalDistance);
            if ( fp.mPlane.D < 0.f ) {
                fp.mPlane.N = -fp.mPlane.N;
                fp.mPlane.D = -fp.mPlane.D;
            }
            float fdist = MAXDISTANCE;
            fp.Intersect(r, fdist);
            Vector fpos = mOrigin + dir * fdist;
            Plane ep(NULL, -mDOPDir, mOrigin.Length());
            Shape *rShape = NULL;
            for ( int x = -mDOPSamples; x <= mDOPSamples; x++ ) for ( int y = -mDOPSamples; y <= mDOPSamples; y++ ) {
                Vector dopPos = screenPos + ( x + gRandom.Rand() ) * mDOPDy + 
                    ( y + gRandom.Rand() ) * mDOPDz;
                Vector dopDir = fpos - dopPos;
                dopDir.Normalize();
                Ray eray(dopPos, -dopDir);
                float edist = MAXDISTANCE;
                ep.Intersect(eray, edist);
                Vector epos = dopPos - dopDir * (edist - EPSILON);

                Ray dopRay(epos, dopDir);
                Color dopacc(0, 0, 0);
                float dist = MAXDISTANCE;
                Shape *s = RayTrace(dopRay, dopacc, 1, dist, 1.f);
                if ( x == 0 && y == 0 ) rShape = s;
                acc += dopacc * (1.f / mDOPTotalSamples);
            }
            return rShape;
        } else {
            float dist = MAXDISTANCE;
            return RayTrace(r, acc, 1, dist, 1.f);
        }
    }

    bool Engine::Render(int32_t width0, int32_t width1, int32_t height1, int32_t height2)
    {
        stringstream ss;

        float invSamples = 1.f / confSSAASamples;
        float invAcc = invSamples * invSamples;
        Vector dxs = mDy * invSamples;
        Vector dys = mDz * invSamples;
        
		for ( int z = height1; z < height2; z++ ) {
            Vector typos = mP1 + (float) z * mDz;
			for ( int y = width0; y < width1; y++ ) {
                if ( !engineOkay ) return false;
                Color acc(0, 0, 0);
				
                if ( confEnableSSAA ) {
                    Vector pos = mP1 + (z - 0.5f) * mDz + (y - 0.5f) * mDy;
                    for ( int32_t xx = 0; xx < confSSAASamples; xx++ ) {
                        for ( int32_t yy = 0; yy < confSSAASamples; yy++ ) {
                            Vector tpos = pos + (float) xx * dxs + (float) yy * dys;
                            RenderRay(tpos, acc);
                        }
                    }
                    acc *= invAcc;
                } else if ( confEnableAdaptiveSampling) {
                    float offsets[8];
                    RequestSamples(offsets, 2);
                    Color samples[4];
                    for ( int i = 0; i < 4; i++ ) {
                        Vector atpos = mP1 + (y - 0.5f + offsets[i*2]) * mDy
                                           + (z - 0.5f + offsets[i*2+1]) * mDz;
                        RenderRay(atpos, samples[i]);
                    }
                    if ( NeedSupersampling(samples, 4, 0.5f) ) {
                        float offsets1[32];
                        RequestSamples(offsets1, 4);
                        Color samples1[16];
                        Color result;
                        for ( int i = 0; i < 16; i++ ) {
                            Vector atpos = mP1 + (y - 0.5f + offsets1[i*2]) * mDy
                                               + (z - 0.5f + offsets1[i*2+1]) * mDz;
                            RenderRay(atpos, samples1[i]);
                            acc += samples1[i];
                        }
                        acc *= (1.f / 16.f);
                    } else {
                        for ( int i = 0; i < 4; i++ ) {
                            acc += samples[i];
                        }
                        acc *= 0.25f;
                    }
                } else {
                    Vector tpos = typos + (float) y * mDy;
                    RenderRay( tpos, acc );
                }
                uint32_t pos = (mHeight - 1 - z) * mWidth + (mWidth - 1 - y);
                _ASSERT(renderTarget);
                renderTarget->SetPixel(pos, acc);

                AtomicAdd(&renderedPixels, 1);
			}
		}

		return true;
    }

    float Engine::CalcShade(Light *light, const Vector &ip, Vector &dir, float samples)
    {
        float retval = 0;
        Shape *prim = NULL;
        if ( light->GetType() == POINT_LIGHT || 
             light->GetType() == SPOT_LIGHT ||
            ( !confEnableSoftShadow && light->GetType() == AREA_LIGHT )) {

            retval = 1.0f;
            dir = light->GetCentroid() - ip;
            float tdist = dir.Length();
            float ldist = tdist;
            dir.Normalize();
            Ray r(ip + dir * EPSILON, dir);
            FindNearest(r, tdist, prim);
            if ( tdist < ldist ) retval = 0.f;
        } else if ( light->GetType() == AREA_LIGHT ) {
            retval = 0.f;
            AreaLight *al = (AreaLight *) light;
            dir = al->GetCentroid() - ip;
            dir.Normalize();
            int s = (int) (confSoftShadowSamples);
            if ( s < 1 ) s = 1;
            float dx = al->mSize.x * (1.f / s);
            float dy = al->mSize.y * (1.f / s);
            float dz = al->mSize.z * (1.f / s);
            
            float reti = 1.f / (s * s * s);
            for ( int x = 0; x < s; x++ ) for ( int y = 0; y < s; y++ ) for ( int z = 0; z < s; z++ ) {
                Vector lp(al->mPosition.x + dx * x + dx * gRandom.Rand(),
                          al->mPosition.y + dy * y + dy * gRandom.Rand(), 
                          al->mPosition.z + dz * z + dz * gRandom.Rand());
                Vector d = lp - ip;
                float tdist = d.Length();
                float ldist = tdist;
                d.Normalize();
                Ray r(ip + d * EPSILON, d);
                FindNearest(r, tdist, prim);
                if ( tdist == ldist ) retval += reti;
            }
        } else if ( light->GetType() == VIRTUAL_LIGHT ) {
            retval = 1.f;
        } else {
            _ASSERT(0);
        }
        return retval;
    }

    Color Engine::EvaluateVolumetricLighting(const Ray &ray, float dist)
    {
        float T0 = 0, T1 = confVolumetricLightingMaximumDistance;
        Shape *temp;
        Color c(0, 0, 0);
        float tt = MAXDISTANCE;
        if ( !mVolumeRange.Intersect(ray, tt) ) return c;

        if ( mVolumeRange.Contains(ray.mOrigin) ) {
            T0 = 0;
            T1 = min(tt, confVolumetricLightingMaximumDistance);
        } else {
            T0 = tt;
            tt = MAXDISTANCE;
            Ray ray1(ray.mOrigin + (T0 + EPSILON) * ray.mDirection, ray.mDirection);
            if ( !mVolumeRange.Intersect(ray1, tt) ) {
                T1 = confVolumetricLightingMaximumDistance;
            } else {
                T1 = min(tt, confVolumetricLightingMaximumDistance);
            }
        }
        if ( T0 > dist ) return c;
        if ( T1 > dist ) T1 = dist;
        float step = confVolumetricLightingSampleStep;
        int32_t nSamples = (int32_t) floorf((T1 - T0) / step);
        float tOffset = gRandom.Rand() * step;
        Vector pos = ray.mOrigin + (T0 + tOffset) * ray.mDirection;
        for ( int i = 0; i < nSamples; i++ ) {
            for ( LightIter iter = mScene->mLights.begin(); iter != mScene->mLights.end(); iter++ ) {
                Light *light = *iter;
                Color lColor = light->Sample(pos);
                if ( lColor.IsBlack() ) continue;
                Vector dir = light->GetCentroid() - pos;
                float tdist = dir.Length();
                float ldist = tdist;
                dir.Normalize();
                Ray r(pos, dir);
                FindNearest(r, tdist, temp);
                if ( tdist < ldist ) continue;
                c += lColor;
            }
            pos += ray.mDirection * step;
        }
        float nDiv = confVolumetricLightingEnhancer * confVolumetricLightingSampleStep;
        c *= nDiv;
        return c;
    }

    float Engine::EvaluateAmbientOcclusion(const Vector &pi, const Vector &N, int nSamples)
    {
        int validSample = 0;
        for ( int i = 0; i < nSamples; i++ ) {
            Vector dir;
            while ( true ) {
                dir = gRandom.RandUnitVector();
                if ( dir.Dot(N) > 0.f ) break;
            }
            float dist = confAmbientOcclusionDistance;
            Shape *shape = NULL;
            IntersectResult iresult = mScene->FindNearest(Ray(pi + N * EPSILON, dir), dist, shape);
            if ( iresult == IR_MISS ) validSample++;
        }
        return static_cast<float>(validSample) / static_cast<float>(nSamples);
    }

    Shape *Engine::RayTrace(const LyraeFX::Ray &ray, LyraeFX::Color &acc, int depth, float dist, float samples)
    {        
        if ( depth > confTraceDepth ) return NULL;
        dist = MAXDISTANCE;

        Vector pi;
        Shape *prim = NULL;
        IntersectResult iresult = FindNearest(ray, dist, prim);

        if ( depth == 1 && confEnableVolumetricLighting ) {
            acc += EvaluateVolumetricLighting(ray, dist);
        }

        if ( iresult == IR_MISS ) return NULL;

        pi = ray.mOrigin + ray.mDirection * dist;
        Vector N = prim->GetNormal(pi) * (float) iresult;
        LightIter lIter;
        Color color = prim->GetColor(pi);

        // evaluate photon mapping
        acc += photonMap.EvaluatePhotonMap(color * (-ray.mDirection.Dot(N)), prim->mMaterial, pi, N);

        // evaluate ambient occlusion
        if ( depth == 1 && confEnableAmbientOcclusion ) {
            float ao = EvaluateAmbientOcclusion(pi, N, confAmbientOcclusionSamples);
            Color c = color * confAmbientOcclusionColor * (ao * confAmbientOcclusionEnhancer);
            acc += c;
        }

        // E
        acc += color * prim->mMaterial->EvaluateE(-ray.mDirection, N);

        for ( lIter = mScene->mLights.begin(); lIter != mScene->mLights.end(); lIter++ ) {
            Light *light = *lIter;
            Color lightColor = light->Sample(pi);
            if ( light->GetType() == VIRTUAL_LIGHT ) {
                if ( ((VirtualLight *) light)->mShapePtr == prim ) continue;
            }
            Vector L = (light->GetCentroid() - pi).GetNormalizedVector();
            if ( lightColor.IsBlack() ) continue;
            Vector dir(Vector(0, 0, 0));
            float shade = 1.0f;
            Color matColor = prim->mMaterial->EvaluateF(-ray.mDirection, L, N);
            if ( matColor.IsBlack() ) continue;
            if ( confEnableShadow ) {
                shade = CalcShade(light, pi, dir, samples);
            }
            if ( shade <= 0.f ) continue;

            // F
            acc += shade * color * lightColor * matColor;
            
        } // for

        for ( uint32_t i = 0; i < prim->mMaterial->BSDFs.size(); i++ ) {
            BSDF *bsdf = prim->mMaterial->BSDFs[i];
            Vector wo;
            Vector N = prim->GetNormal(pi);
            Color r(0, 0, 0), c(0, 0, 0);

            // avoid glossy BSDF tracing endlessly
            if ( bsdf->MatchesFlags(BSDFType(BSDF_GLOSSY | BSDF_REFLECTION)) && depth > 1 ) 
                continue;
            if ( bsdf->MatchesFlags(BSDFType(BSDF_GLOSSY | BSDF_TRANSMISSION)) && depth > 2 ) 
                continue;

            // G
            Vector *vos = NULL;
            int voNum = 0;
            if ( !bsdf->G(-ray.mDirection, vos, voNum, N, r, samples) ) continue;
            _ASSERT(voNum > 0);
            float invVoNum = 1.f / static_cast<float>(voNum);
            for ( int i = 0; i < voNum; i++ ) {
                float dist = MAXDISTANCE;
                Color r1(0, 0, 0);
                RayTrace(Ray(pi + vos[i] * EPSILON, vos[i]), r1, depth+1, dist, 
                    samples * confImpSamples);
                c += r1;
            }
            acc += r * c * color * invVoNum;
            delete [] vos;
        }

        return prim;
    }

    Engine gEngine;
} // namespace LyraeFX