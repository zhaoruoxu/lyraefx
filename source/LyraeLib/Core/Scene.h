#if defined(_MSC_VER)
#pragma once
#endif

#ifndef LYRAE_CORE_SCENE_H
#define LYRAE_CORE_SCENE_H

#include "LyraeFX.h"
#include "Shape.h"
#include "Geometry.h"
#include "Texture.h"
#include "Light.h"
#include "BVH.h"
#include "Shading.h"
#include "../TinyXml/tinyxml.h"
#include "XmlParse.h"

namespace LyraeFX
{
    typedef vector<Shape *>::iterator ShapeIter;
    typedef vector<Light *>::iterator LightIter;
    typedef map<string, Texture *>::iterator TextureIter;
    typedef map<string, Material *>::iterator MaterialIter;

    class Scene 
    {
    public:

        Scene() : bvhAccel(NULL), mEngine(NULL), mDOPSamples(0), mDOPRange(1.f),
            mDOPFocalDistance(0.f), mDOPEnabled(false), isInitialized(false) {}
        virtual ~Scene() { Reset(); }

        bool InitScene(string sceneName);
        void Reset();
        bool ReloadScene() { return InitScene(filename); }
        bool IsInitialized() { return isInitialized; }
        void AddShape(Shape *shape) { mShapes.push_back(shape); }
        void AddLight(Light *light) { mLights.push_back(light); }
        void AddTexture(const string &name, Texture *texture) { mTextures[name] = texture; }
        void AddMaterial(const string &name, Material *material) { mMaterials[name] = material; }
        bool LoadSceneFromXml(const char *filename);
        void LoadConfig();

        inline IntersectResult FindNearest(const Ray &ray, float &dist, Shape *&shape);
 
        Engine *mEngine;
        vector<Shape *> mShapes;
        vector<Light *> mLights;
        map<string, Texture *> mTextures;
        map<string, Material *> mMaterials;
        int32_t mDOPSamples;
        float mDOPRange;
        float mDOPFocalDistance;
        bool mDOPEnabled;
        BVHAccelerator *bvhAccel;
        string filename;

    protected:
        bool isInitialized;
        bool LoadLyraeRoot(TiXmlElement *root);
        bool LoadScene(TiXmlElement *scene);
        bool LoadMaterials(TiXmlElement *materials);
        bool LoadTextures(TiXmlElement *textures);
        bool LoadShapes(TiXmlElement *shapes);
        bool LoadLights(TiXmlElement *lights);

        // shapes
        bool LoadShape(TiXmlElement *shape);
        bool LoadBox(TiXmlElement *shape, Material *m);
        bool LoadSphere(TiXmlElement *shape, Material *m);
        bool LoadCylinder(TiXmlElement *shape, Material *m);
        bool LoadTCylinder(TiXmlElement *shape, Material *m);
        bool LoadPlane(TiXmlElement *shape, Material *m);
        bool LoadTriangleMesh(TiXmlElement *shape, Material *m);
        bool LoadTriangleMeshFromFile(const char *file, Material *m, float scale);

        // lights
        bool LoadLight(TiXmlElement *light);
        bool LoadPointLight(TiXmlElement *light, const Vector &color, float intensity, float alpha);
        bool LoadAreaLight(TiXmlElement *light, const Vector &color, float intensity, float alpha);
        bool LoadSpotLight(TiXmlElement *light, const Vector &color, float intensity, float alpha);

        // BSDFs
        bool LoadBSDFs(TiXmlElement *bsdf,  Material *material);
        bool LoadLambertian(TiXmlElement *lambertian, BSDF *&bsdf);
        bool LoadPhong(TiXmlElement *phong, BSDF *&bsdf);
        bool LoadSpecularReflection(TiXmlElement *specrefl, BSDF *&bsdf);
        bool LoadSpecularTransmission(TiXmlElement *spectran, BSDF *&bsdf);
        bool LoadFresnel(TiXmlElement *fresnel, Fresnel *&f);
        bool LoadSimpleLight(TiXmlElement *light, BSDF *&bsdf);
        bool LoadGlossyReflection(TiXmlElement *glosrefl, BSDF *&bsdf);
        bool LoadGlossyTransmission(TiXmlElement *glostran, BSDF *&bsdf);
    }; // class Scene

    inline IntersectResult Scene::FindNearest(const Ray &ray, float &dist, Shape *&shape)
    {
        return bvhAccel->FindNearest(ray, dist, shape);
    }
}

#endif // LYRAE_CORE_SCENE_H