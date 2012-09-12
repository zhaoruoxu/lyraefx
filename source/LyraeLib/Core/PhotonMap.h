#if defined(_MSC_VER)
#pragma once
#endif

#ifndef LYRAE_CORE_PHOTONMAP_H
#define LYRAE_CORE_PHOTONMAP_H

#include "LyraeFX.h"
#include "kdtree.h"
#include "Geometry.h"
#include "Scene.h"

namespace LyraeFX
{

    class Photon
    {
    public:
        Photon(const Color &c, const Vector &dir)
            :mColor(c), mDirection(dir) {}

        Color mColor;
        Vector mDirection;
    };

    class PhotonPos
    {
    public:
        PhotonPos(Photon *p, const Vector &po, float d)
            :photon(p), pos(po), distSquared(d)
        {
        }

        bool operator<(const PhotonPos &p2) const
        {
            return distSquared < p2.distSquared;
        }

        Photon *photon;
        Vector pos;
        float distSquared;
    };

    enum PhotonBehavior 
    { 
        PHOTON_DIFFUSE, 
        PHOTON_SPECULAR, 
        PHOTON_TRANSMISSION, 
        PHOTON_ABSORPTION 
    };

    class PhotonMap
    {
    public:
        PhotonMap(Scene *s, LPCRITICAL_SECTION cs) : scene(s), lpcs(cs) 
        {
            causticsMap = NULL;
            indirectMap = NULL;
            volumeMap = NULL;
            causticsCount = 0;
            indirectCount = 0;
            volumeCount = 0;
            _ASSERT(s);
            _ASSERT(cs);
        }

        void AddPhoton(kdtree *map, const Vector &pos, Photon *photon);
        void InitPhotonMap();
        void CleanupPhotons();
        void LoadConfig();
        void ThreadBuildPhotonMap(uint32_t thdNum);
        Color EvaluatePhotonMap(const Color &materialColor, Material *material, const Vector &pos, const Vector &N);


        int32_t causticsCount;
        int32_t indirectCount;
        int32_t volumeCount;

    protected:
        Scene *scene;
        vector<Photon *> photons;
        kdtree *causticsMap;
        kdtree *indirectMap;
        kdtree *volumeMap;
        LPCRITICAL_SECTION lpcs;

        bool confEnablePhotonMapping;
        bool confEnablePhotonCaustics;
        bool confEnablePhotonIndirect;
        bool confEnablePhotonVolumetric;

        int confPhotonsCausticsPerLight;
        int confPhotonsIndirectPerLight;
        float confPhotonAbsorptionProbability;
        float confPhotonCausticsEnhancer;
        float confPhotonIndirectEnhancer;
        float confPhotonCausticsRadius;
        float confPhotonIndirectRadius;
        int confPhotonCausticsLookupMaximum;
        int confPhotonCausticsLookupMinimum;
        int confPhotonIndirectLookupMaximum;
        int confPhotonIndirectLookupMinimum;
        int confPhotonIndirectMaximumDepth;
        int confPhotonCausticsMaximumDepth;
        bool confPhotonMappingDebugView;
        float confPhotonMappingDebugRadius;
        bool confEnableDirectedPhotons;
        Vector confPhotonDirectedP1;
        Vector confPhotonDirectedP2;

        void BuildCausticsPhotonMap(int32_t n);
        void BuildIndirectPhotonMap(int32_t n);
        void BuildVolumetricPhotonMap(int32_t n);
        void ShootIndirectPhoton(const Vector &dir, const Vector &pi, const Color &color, Shape *shape, int depth);
        void ShootCausticsPhoton(const Vector &dir, const Vector &pi, const Color &color, Shape *shape, int depth);
        Color EvaluateCausticRadiance(kdtree *map, const Color &materialColor, const Vector &pos, const Vector &N, float radius);
        Color EvaluateIndirectRadiance(kdtree *map, const Color &materialColor, const Vector &pos, const Vector &N, float radius);
        Color EvaluateVolumetricRadiance(kdtree *map, const Vector &pos, float radius);
        int32_t SelectNNearestPhotons(kdtree *map, PhotonPos *pp, const Vector &pos, float radius, int32_t n);

    };
}


#endif // LYRAE_CORE_PHOTONMAP_H