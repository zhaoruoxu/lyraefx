#include "PhotonMap.h"
#include "../Config/EngineConfig.h"
#include "Engine.h"
#include "Error.h"

namespace LyraeFX
{
    struct PhotonAndDist
    {
        PhotonAndDist(Photon *p, float d)
            : photon(p), dist(d) {}

        Photon *photon;
        float dist;
    };

    bool PhotonDistCompare(PhotonAndDist *a, PhotonAndDist *b)
    {
        return a->dist < b->dist;
    }


    void PhotonMap::LoadConfig()
    {
        confEnablePhotonMapping         = gEngineConfig.GetBool("EnablePhotonMapping");
        confEnablePhotonCaustics        = gEngineConfig.GetBool("EnablePhotonCaustics");
        confEnablePhotonIndirect        = gEngineConfig.GetBool("EnablePhotonIndirect");
        confEnablePhotonVolumetric      = gEngineConfig.GetBool("EnablePhotonVolumetric");
        confPhotonsCausticsPerLight     = gEngineConfig.GetInt32("PhotonsCausticsPerLight");
        confPhotonsIndirectPerLight     = gEngineConfig.GetInt32("PhotonsIndirectPerLight");
        confPhotonAbsorptionProbability = gEngineConfig.GetFloat("PhotonAbsorptionProbability");
        confPhotonCausticsEnhancer      = gEngineConfig.GetFloat("PhotonCausticsEnhancer");
        confPhotonIndirectEnhancer      = gEngineConfig.GetFloat("PhotonIndirectEnhancer");
        confPhotonCausticsRadius        = gEngineConfig.GetFloat("PhotonCausticsRadius");
        confPhotonIndirectRadius        = gEngineConfig.GetFloat("PhotonIndirectRadius");
        confPhotonCausticsLookupMaximum = gEngineConfig.GetInt32("PhotonCausticsLookupMaximum");
        confPhotonCausticsLookupMinimum = gEngineConfig.GetInt32("PhotonCausticsLookupMinimum");
        confPhotonIndirectLookupMaximum = gEngineConfig.GetInt32("PhotonIndirectLookupMaximum");
        confPhotonIndirectLookupMinimum = gEngineConfig.GetInt32("PhotonIndirectLookupMinimum");
        confPhotonIndirectMaximumDepth  = gEngineConfig.GetInt32("PhotonIndirectMaximumDepth");
        confPhotonCausticsMaximumDepth  = gEngineConfig.GetInt32("PhotonCausticsMaximumDepth");
        confPhotonMappingDebugView      = gEngineConfig.GetBool("PhotonMappingDebugView");
        confPhotonMappingDebugRadius    = gEngineConfig.GetFloat("PhotonMappingDebugRadius");
        confEnableDirectedPhotons       = gEngineConfig.GetBool("EnableDirectedPhotons");
        confPhotonDirectedP1            = gEngineConfig.GetVector("PhotonDirectedP1");
        confPhotonDirectedP2            = gEngineConfig.GetVector("PhotonDirectedP2");
    }

    void PhotonMap::InitPhotonMap()
    {
        causticsCount = 0;
        indirectCount = 0;
        volumeMap = 0;
        if ( confEnablePhotonMapping ) {
            if ( confEnablePhotonCaustics ) { causticsMap = kd_create(3); } 
            else { causticsMap = NULL; }
            if ( confEnablePhotonIndirect ) { indirectMap = kd_create(3); } 
            else { indirectMap = NULL; }
            if ( confEnablePhotonVolumetric ) { volumeMap = kd_create(3); } 
            else { volumeMap = NULL; }
        }
    }

    void PhotonMap::AddPhoton(kdtree *map, const Vector &pos, Photon *photon)
    {
        _ASSERT(photon);
        _ASSERT(map);
        EnterCriticalSection(lpcs);
        kd_insert3f(map, pos.x, pos.y, pos.z, photon);
        photons.push_back(photon);
        if ( map == causticsMap ) causticsCount++;
        else if ( map == indirectMap ) indirectCount++;
        else if ( map == volumeMap ) volumeCount++;
        LeaveCriticalSection(lpcs);
    }

    void PhotonMap::CleanupPhotons()
    {
        if ( causticsMap ) {
            kd_free(causticsMap);
            causticsMap = NULL;
        }
        if ( indirectMap ) {
            kd_free(indirectMap);
            indirectMap = NULL;
        }
        if ( volumeMap ) {
            kd_free(volumeMap);
            volumeMap = NULL;
        }
        for ( uint32_t i = 0; i < photons.size(); i++ ) { 
            delete photons[i]; 
        }
        photons.clear();
    }

    void PhotonMap::ThreadBuildPhotonMap(uint32_t thdNum)
    {
        stringstream ss;
        int32_t threads = gEngineConfig.GetInt32("RenderThreads");
        int32_t causticsPhotons = confPhotonsCausticsPerLight / threads;
        int32_t indirectPhotons = confPhotonsIndirectPerLight / threads;

        if ( confEnablePhotonCaustics ) {
            ss << "[Thread " << thdNum << "] Building caustics photons map, target number: " << causticsPhotons;
            Info(ss);

            BuildCausticsPhotonMap(causticsPhotons);

            ss << "[Thread " << thdNum << "] Caustics photons: " << causticsCount;
            Info(ss); 
        }

        if ( confEnablePhotonIndirect ) {
            ss << "[Thread " << thdNum << "] Building indirect photons map, target number: " << indirectPhotons;
            Info(ss); 

            BuildIndirectPhotonMap(indirectPhotons);
            ss << "[Thread " << thdNum << "] Indirect photons: " << indirectCount;
            Info(ss);
        }

        ss << "[Thread " << thdNum << "] Photons: " << photons.size() << "  Caustics: " << causticsCount << "  Indirect: "
            << indirectCount << "  Volumetric: " << volumeCount;
        Info(ss);
    }



    //
    //  dir      - incoming direction
    //  pi       - hitting point
    //  color    - ray(photon)'s color
    //  shape    - shape to hit
    //  depth    - recursive depth
    //
    void PhotonMap::ShootIndirectPhoton(const Vector &dir, const Vector &pi, const Color &color, 
        Shape *shape, int depth)
    {
        if ( depth != 0 ) {
            Photon *photon = new Photon(color, dir);
            AddPhoton(indirectMap, pi, photon);
        }

        if ( depth < confPhotonIndirectMaximumDepth ) {
            if ( !shape->mMaterial->MatchesBSDFFlags(BSDF_DIFFUSE) )
                return;
            Vector N = shape->GetNormal(pi);
            if ( N.Dot(dir) > 0 ) N = -N;
            Vector newDir;
            while ( true ) {
                newDir = gRandom.RandUnitVector();
                if ( newDir.Dot(N) > 0 ) break;
            }
            Color matColor = shape->mMaterial->EvaluateF(dir, newDir, N) * shape->GetColor(pi);
            if ( matColor.IsBlack() ) return;
            float dist = MAXDISTANCE;
            Shape *newShape;
            if ( scene->FindNearest(Ray(pi + N * EPSILON, newDir), dist, newShape) == IR_MISS )
                return;
            ShootIndirectPhoton(newDir, pi + newDir * dist, color * matColor, newShape, depth + 1);
        }
    }

    void PhotonMap::ShootCausticsPhoton(const Vector &dir, const Vector &pi, const Color &color, Shape *shape, int depth)
    {
        if ( depth >= confPhotonCausticsMaximumDepth ) return;
        if ( depth != 0 && shape->mMaterial->MatchesBSDFFlags(BSDF_DIFFUSE) ) {
            // Diffuse material, add photon
            Photon *photon = new Photon(color, dir);
            AddPhoton(causticsMap, pi, photon);
        } else if ( shape->mMaterial->MatchesBSDFFlags(BSDF_SPECULAR) ) {
            // Specular material, continue build photon
            Vector N = shape->GetNormal(pi);
            for ( uint32_t i = 0; i < shape->mMaterial->BSDFs.size(); i++ ) {
                BSDF *bsdf = shape->mMaterial->BSDFs[i];
                if ( !bsdf->MatchesFlags(BSDFType(BSDF_SPECULAR)) ) continue;
                Vector newDir;
                Color matColor;
                Vector *vos = NULL;
                int voNum = 0;
                if (  bsdf->G(-dir, vos, voNum, N, matColor, 1.f) ) {
                    _ASSERT(voNum > 0);
                    newDir = vos[0];
                    delete [] vos;
                    vos = NULL;
                    matColor *= shape->GetColor(pi);
                    if ( matColor.IsBlack() ) continue;
                    float dist = MAXDISTANCE;
                    Shape *newShape;
                    Vector norm = N.Dot(dir) > 0 ? -N : N;
                    if ( bsdf->MatchesFlags(BSDF_TRANSMISSION) ) norm = -norm;
                    if ( scene->FindNearest(Ray(pi + norm * EPSILON, newDir), dist, newShape) == IR_MISS )
                        continue;
                    ShootCausticsPhoton(newDir, pi + newDir * dist, color * matColor, newShape, depth + 1);
                }
            }

        }
    }

    void PhotonMap::BuildIndirectPhotonMap(int32_t n)
    {
        for ( LightIter iter = scene->mLights.begin(); iter != scene->mLights.end(); iter++ ) {
            Light *light = *iter;
            int32_t nPhotons = (int) (n * light->mIntensity);
            for ( int32_t i = 0; i < nPhotons; i++ ) {
                Color color;
                Vector pi, dir;
                Shape *shape = NULL;

                while ( true ) {
                    if ( confEnableDirectedPhotons ) {
                        Vector r(gRandom.Rand(), gRandom.Rand(), gRandom.Rand());
                        dir = confPhotonDirectedP1 + r * (confPhotonDirectedP2 - confPhotonDirectedP1) 
                            - light->GetCentroid();
                        dir.Normalize();
                    } else {
                        dir = light->RandomDirection(gRandom);
                    }
                    float dist = MAXDISTANCE;
                    if ( scene->FindNearest(Ray(light->GetCentroid(), dir), dist, shape) == IR_MISS ) 
                        continue;
                    pi = light->GetCentroid() + dir * dist;
                    color = light->Sample(pi);
                    if ( !color.IsBlack() ) break;
                }
                ShootIndirectPhoton(dir, pi, color, shape, 0);
            }
        }
    }

    void PhotonMap::BuildCausticsPhotonMap(int32_t n)
    {
        vector<Shape *> causticsShapes;
        uint32_t causticsCount;
        for ( ShapeIter iter = scene->mShapes.begin(); iter != scene->mShapes.end(); iter++ ) {
            Shape *shape = *iter;
            if ( shape->GetType() == SHAPE_TRIANGLE ) continue;
            if ( shape->mMaterial->MatchesBSDFFlags(BSDF_SPECULAR) ) {
                causticsShapes.push_back(shape);
            }
        }
        causticsCount = causticsShapes.size();
        if ( causticsCount == 0 ) {
            Warning("No caustics shape.");
            return;
        }
        for ( LightIter iter = scene->mLights.begin(); iter != scene->mLights.end(); iter++ ) {
            Light *light = *iter;
            int32_t nPhotons = static_cast<int>(n * light->mIntensity);
            for ( int i = 0; i < nPhotons; i++ ) {
                Color color;
                Vector pi;
                Vector dir;
                Shape *shape = NULL;
                while ( true ) {
                    uint32_t shapeNr = static_cast<uint32_t>(gRandom.Rand() * causticsCount);
                    if ( shapeNr == causticsCount ) shapeNr = causticsCount-1;
                    Shape *causticsShape = causticsShapes[shapeNr];
                    AABB box = causticsShape->GetAABB();
                    Vector r(gRandom.Rand(), gRandom.Rand(), gRandom.Rand());
                    dir = box.mPos + box.mSize * r - light->GetCentroid();
                    dir.Normalize();
                    float dist = MAXDISTANCE;
                    if ( scene->FindNearest(Ray(light->GetCentroid(), dir), dist, shape) == IR_MISS )
                        continue;
                    pi = light->GetCentroid() + dir * dist;
                    color = light->Sample(pi);
                    if ( !color.IsBlack() ) break;
                }
                ShootCausticsPhoton(dir, pi, color, shape, 0);
            }
        }
    }

    void PhotonMap::BuildVolumetricPhotonMap(int32_t n)
    {
        _ASSERT(!"Volumetric photon mapping not implemented");
    }

    Color PhotonMap::EvaluateCausticRadiance(kdtree *map, const Color &materialColor, const Vector &pos, const Vector &N, float radius)
    {
        Color c(0, 0, 0);
        if ( confPhotonMappingDebugView ) {
            radius = confPhotonMappingDebugRadius;
        }

        float invCount = confPhotonCausticsEnhancer / (confPhotonCausticsRadius * causticsCount);
        PhotonPos *pp = (PhotonPos *) malloc(confPhotonCausticsLookupMaximum * sizeof(PhotonPos));
        int n = SelectNNearestPhotons(causticsMap, pp, pos, radius, confPhotonCausticsLookupMaximum);
        if ( n < (confPhotonMappingDebugView ? 1 :  n < confPhotonCausticsLookupMinimum ) ) {
            free(pp);
            return c;
        }
        float invMaxDist = 1.f / pp[n - 1].distSquared * INV_FOURPI;
        float invRadius2 = 1.f / (radius * radius);
        for ( int i = 0; i < n; i++ ) {
            if ( confPhotonMappingDebugView ) {
                c += pp[i].photon->mColor;
            } else {
                float dot = -N.Dot(pp[i].photon->mDirection);
                if ( dot > 0 ) {
                    float x = 1 - pp[i].distSquared * invRadius2;
                    Color delta = (invCount * x) * pp[i].photon->mColor;
                    float coneFilter = 1.f - sqrtf(pp[i].distSquared) / radius;
                    c += materialColor * delta;
                }
            }
        }
        if ( confPhotonMappingDebugView ) {
            c *= (1.f / n);
        }
        free(pp);
        return c;
    }

    Color PhotonMap::EvaluateIndirectRadiance(kdtree *map, const Color &materialColor, const Vector &pos, const Vector &N, float radius)
    {
        Color c(0, 0, 0);
        if ( confPhotonMappingDebugView ) {
            radius = confPhotonMappingDebugRadius;
        }
        PhotonPos *pp = (PhotonPos *) malloc(confPhotonIndirectLookupMaximum * sizeof(PhotonPos));
        int n = SelectNNearestPhotons(indirectMap, pp, pos, radius, confPhotonIndirectLookupMaximum);

        if ( n < (confPhotonMappingDebugView ? 1 : confPhotonIndirectLookupMinimum) ) {
            free(pp);
            return c;
        }

        float invRadius2 = 1.f / (radius * radius);
        float invCount = confPhotonIndirectEnhancer / (radius * indirectCount);
        for ( int i = 0; i < n; i++ ) {
            if ( confPhotonMappingDebugView ) {
                c += pp[i].photon->mColor;
            } else {
                float x = 1 - pp[i].distSquared / invRadius2;

                float dot = -N.Dot(pp[i].photon->mDirection);
                if ( dot > 0 ) {
                    float x = 1 - pp[i].distSquared * invRadius2;
                    Color delta = (invCount * dot * x) * pp[i].photon->mColor;
                    c += materialColor * delta;
                }
            }
        }
        if ( confPhotonMappingDebugView ) {
            c *= (1.f / n);
        }
        free(pp);
        return c;
    }

    Color PhotonMap::EvaluateVolumetricRadiance(kdtree *map, const Vector &pos, float radius)
    {
        _ASSERT(!"Volumetric photon mapping not implemented");
        return Color();
    }

    int32_t PhotonMap::SelectNNearestPhotons(kdtree *map, PhotonPos *pp, const Vector &pos, float radius, int32_t n)
    {
        kdres *result = kd_nearest_range3f(map, pos.x, pos.y, pos.z, radius);
        Vector ppos;
        int nNow = 0;
        while ( !kd_res_end(result) ) {
            Photon *photon = (Photon *) kd_res_itemf(result, &(ppos.x));
            float d = (ppos - pos).LengthSquared();

            if ( nNow < n ) {
                pp[nNow++] = PhotonPos(photon, ppos, d);
                if ( nNow == n ) make_heap(pp, pp + n);
            }
            else {
                pop_heap(pp, pp + n);
                pp[n - 1] = PhotonPos(photon, ppos, d);
                push_heap(pp, pp + n);
            }
            kd_res_next(result);
        }
        kd_res_free(result);

        return nNow;
    }

    Color PhotonMap::EvaluatePhotonMap(const Color &materialColor, Material *material, const Vector &pos, const Vector &N)
    {
        Color c(0, 0, 0);
        if ( confEnablePhotonMapping )
        {
            if ( confEnablePhotonCaustics )
                c += EvaluateCausticRadiance(causticsMap, materialColor, pos, N, confPhotonCausticsRadius);
            if ( confEnablePhotonIndirect && material->MatchesBSDFFlags(BSDF_DIFFUSE) )
                c += EvaluateIndirectRadiance(indirectMap, materialColor, pos, N, confPhotonIndirectRadius);
            if ( confEnablePhotonVolumetric ) {
                Warning("Volumetric photon mapping is not implemented");
                c += EvaluateVolumetricRadiance(volumeMap, pos, 0.05f);
            }
        }
        return c;
    }
}