#include "Scene.h"
#include "Material.h"
#include "Light.h"
#include "Texture.h"
#include "Engine.h"
#include "tinyxml.h"
#include "Config.h"
#include "XmlParse.h"
#include "Error.h"
#include "lib3ds.h"
#include "Shading.h"

#include "../Shape/Box.h"
#include "../Shape/Plane.h"
#include "../Shape/Sphere.h"
#include "../Shape/TCylinder.h"
#include "../Shape/Cylinder.h"
#include "../Shape/TriangleMesh.h"

#include "../Light/PointLight.h"
#include "../Light/AreaLight.h"
#include "../Light/SpotLight.h"

#include "../Config/EngineConfig.h"

#include "../PostProcessor/HDRPostProcessor.h"

namespace LyraeFX
{

    bool Scene::InitScene(string sceneName)
	{
        isInitialized = false;
        filename = sceneName;
        Reset();
        Info("Loading scene from: \"" + sceneName + "\"");
        bool loadOkay = LoadSceneFromXml(sceneName.c_str());
        if ( !loadOkay ) return false;

        bvhAccel = new BVHAccelerator(mShapes, 1, BVH_SAH);
        isInitialized = true;
        if ( gEngineConfig.GetBool("Verbose") ) {
            stringstream ss;
            ss << "Scene loaded: Shapes[" << mShapes.size() << "] "
                << " Lights [" << mLights.size() << "] "
                << " Textures [" << mTextures.size() << "] "
                << " Materials [" << mMaterials.size() << "]";
            Info(ss);
        }
        return true;
	}

    void Scene::LoadConfig()
    {
        mDOPEnabled = gEngineConfig.GetBool("DOPEnabled");
        mDOPFocalDistance = gEngineConfig.GetFloat("DOPFocalDistance");
        mDOPRange = gEngineConfig.GetFloat("DOPRange");
        mDOPSamples = gEngineConfig.GetInt32("DOPSamples");
    }

    void Scene::Reset()
    {
        for ( ShapeIter iter = mShapes.begin(); iter != mShapes.end(); iter++ ) {
            delete *iter;
        }
        for ( LightIter iter = mLights.begin(); iter != mLights.end(); iter++ ) {
            delete *iter;
        }
        for ( TextureIter iter = mTextures.begin(); iter != mTextures.end(); iter++ ) {
            delete iter->second;
        }
        for ( MaterialIter iter = mMaterials.begin(); iter != mMaterials.end(); iter++ ) {
            delete iter->second;
        }
        mShapes.clear();
        mLights.clear();
        mTextures.clear();
        mMaterials.clear();
        if ( bvhAccel ) {
            delete bvhAccel;
            bvhAccel = NULL;
        }
    }

    bool Scene::LoadSceneFromXml(const char *filename)
    {
        Reset();

        TiXmlDocument xmlDoc(filename);
        bool loadOkay = xmlDoc.LoadFile();
        if ( !loadOkay ) return false;

        TiXmlHandle docHandle(&xmlDoc);
        TiXmlElement *fxRoot = docHandle.FirstChild("LyraeDesc").ToElement();
        if ( NULL == fxRoot ) {
            stringstream ss;
            ss << "Invalid description file: " << filename;
            Severe(ss);
            return false;
        } else {
            return LoadLyraeRoot(fxRoot);
        }
    }

    bool Scene::LoadLyraeRoot(TiXmlElement *root)
    {
        TiXmlHandle rootHandle(root);
        TiXmlElement *sceneElem = rootHandle.FirstChild("Scene").ToElement();
        if ( sceneElem ) {
            if ( !LoadScene(sceneElem) ) return false;
        } else {
            Severe("Missing scene description");
        }
        TiXmlElement *configElem = rootHandle.FirstChild("Config").ToElement();
        if ( configElem ) {
            if ( !XmlParseConfig(gEngineConfig, configElem) ) {
                return false;
            }
        }
        return true;
    }

    bool Scene::LoadScene(TiXmlElement *scene)
    {
        TiXmlHandle sceneHandle(scene);
        TiXmlElement *textures = sceneHandle.FirstChild("Textures").ToElement();
        if ( textures ) {
            if ( !LoadTextures(textures) ) return false;
        }
        TiXmlElement *materials = sceneHandle.FirstChild("Materials").ToElement();
        if ( materials ) {
            if ( !LoadMaterials(materials) ) return false;
        }
        TiXmlElement *shapes = sceneHandle.FirstChild("Shapes").ToElement();
        if ( shapes ) {
            if ( !LoadShapes(shapes) ) return false;
        }
        TiXmlElement *lights = sceneHandle.FirstChild("Lights").ToElement();
        if ( lights ) {
            if ( !LoadLights(lights) ) return false;
        }
        return true;
    }

    bool Scene::LoadMaterials(TiXmlElement *materials)
    {
        if ( gEngineConfig.GetBool("Verbose") ) {
            Info("Loading materials");
        }
        TiXmlHandle materialsHandle(materials);
        TiXmlElement *material = materialsHandle.FirstChild("Material").ToElement();
        for ( ; material; material = material->NextSiblingElement("Material") ) {
            Color color(1, 1, 1);
            TiXmlElement *colorElem = (TiXmlElement *) material->FirstChild("Color");
            if ( colorElem ) {
                if ( !XmlParseColor(color, colorElem) ) return false;
            }
            const char *name = material->Attribute("Name");
            if ( !name ) {
                Severe("Material's Name attribute missing");
                return false;
            }
            Material *pMaterial = new Material;
            pMaterial->mColor = color;
            double value;
            if ( material->Attribute("UScale", &value) ) pMaterial->mUScale = (float) value;
            if ( material->Attribute("VScale", &value) ) pMaterial->mVScale = (float) value;
            const char *texture = material->Attribute("Texture");
            if ( texture ) {
                string textureName(texture);
                pMaterial->mTexture = mTextures[textureName];
                if ( !pMaterial->mTexture ) {
                    stringstream ss;
                    ss << "Texture not found: [" << texture << "] in material [" << name << "]";
                    Warning(ss);
                }
            }
            TiXmlElement *bsdfs = (TiXmlElement *) material->FirstChild("BSDF");
            if ( bsdfs ) {
                if ( !LoadBSDFs(bsdfs, pMaterial) ) return false;
            } else {
                Severe("Material's BSDF not found!");
                delete pMaterial;
                return false;
            }

            string materialName(name);
            mMaterials[materialName] = pMaterial;
            if ( gEngineConfig.GetBool("Verbose") ) {
                stringstream ss;
                ss << "Material loaded: [" << name << "]";
                Info(ss);
            }
        }
        return true;
    }

    bool Scene::LoadBSDFs(TiXmlElement *bsdfs, Material *material)
    {
        stringstream ss;
        TiXmlElement *bsdf = (TiXmlElement *) bsdfs->FirstChild();
        for ( ; bsdf; bsdf = bsdf->NextSiblingElement() ) {
            if ( bsdf->Type() != TiXmlNode::TINYXML_ELEMENT ) continue;
            string bsdfType(bsdf->Value());
            BSDF *value = NULL;
            if ( bsdfType == "Lambertian" ) {
                if ( !LoadLambertian(bsdf, value) ) return false;
            } else if ( bsdfType == "Phong" ) {
                if ( !LoadPhong(bsdf, value) ) return false;
            } else if ( bsdfType == "SpecularReflection" ) {
                if ( !LoadSpecularReflection(bsdf, value) ) return false;
            } else if ( bsdfType == "SpecularTransmission" ) {
                if ( !LoadSpecularTransmission(bsdf, value) ) return false;
            } else if ( bsdfType == "SimpleLight" ) {
                if ( !LoadSimpleLight(bsdf, value) ) return false;
            } else if ( bsdfType == "GlossyReflection" ) {
                if ( !LoadGlossyReflection(bsdf, value) ) return false;
            } else if ( bsdfType == "GlossyTransmission" ) {
                if ( !LoadGlossyTransmission(bsdf, value) ) return false;
            } else {
                ss << "Unknown BSDF type: [" << bsdfType << "]";
                return false;
            }
            material->BSDFs.push_back(value);
        }
        return true;
    }
    bool Scene::LoadLambertian(TiXmlElement *lambertian, BSDF *&bsdf)
    {
        Color c;
        TiXmlElement *colorElem = (TiXmlElement *) lambertian->FirstChild("Color");
        if ( !colorElem ) {
            Severe("Lambertian's Color attribute missing");
            return false;
        }
        if ( !XmlParseColor(c, colorElem) ) return false;
        bsdf = new Lambertian(c);
        return true;
    }

    bool Scene::LoadPhong(TiXmlElement *phong, BSDF *&bsdf)
    {
        double d, s, a;
        if ( !phong->Attribute("Diffuse", &d) ) {
            Severe("Phong's Diffuse attribute missing");
            return false;
        }
        if ( !phong->Attribute("Specular", &s) ) {
            Severe("Phong's Specular attribute missing");
            return false;
        }
        if ( !phong->Attribute("Ambient", &a) ) {
            Severe("Phong's Ambient Attribute missing");
            return false;
        }
        double e = 20;
        phong->Attribute("Exp", &e);
        Color diffuse;
        TiXmlElement *diffuseElem = (TiXmlElement *) phong->FirstChild("DiffuseColor");
        TiXmlElement *specularElem = (TiXmlElement *) phong->FirstChild("SpecularColor");
        TiXmlElement *ambientElem = (TiXmlElement *) phong->FirstChild("AmbientColor");
        if ( !diffuseElem ) {
            Severe("Phong's DiffuseColor attribute missing");
            return false;
        }
        if ( !XmlParseColor(diffuse, diffuseElem) ) return false;
        Color specular(diffuse), ambient(diffuse);
        if ( specularElem ) {
            if ( !XmlParseColor(specular, specularElem) ) return false;
        }
        if ( ambientElem ) {
            if ( !XmlParseColor(ambient, ambientElem) ) return false;
        }
        bsdf = new Phong(diffuse, specular, ambient, (float) d, (float) s, (float) a, (float) e);
        return true;
    }

    bool Scene::LoadSpecularReflection(TiXmlElement *specrefl, BSDF *&bsdf)
    {
        Color c(1, 1, 1);
        TiXmlElement *colorElem = (TiXmlElement *) specrefl->FirstChild("Color");
        if ( colorElem ) {
            if ( !XmlParseColor(c, colorElem) ) return false;
        }
        Fresnel *f;
        TiXmlElement *fresnelElem = (TiXmlElement *) specrefl->FirstChild("Fresnel");
        if ( !fresnelElem ) {
            Severe("SpecularReflection's Fresnel attribute missing");
            return false;
        }
        if ( !LoadFresnel(fresnelElem, f) ) return false;
        bsdf = new SpecularReflection(c, f);
        return true;
    }

    bool Scene::LoadGlossyReflection(TiXmlElement *glosrefl, BSDF *&bsdf)
    {
        Color c(1, 1, 1);
        double scale;
        int samples;
        TiXmlElement *colorElem = (TiXmlElement *) glosrefl->FirstChild("Color");
        if ( colorElem ) {
            if ( !XmlParseColor(c, colorElem) ) return false;
        }
        Fresnel *f;
        TiXmlElement *fresnelElem = (TiXmlElement *) glosrefl->FirstChild("Fresnel");
        if ( !fresnelElem ) {
            Severe("SpecularReflection's Fresnel attribute missing");
            return false;
        }
        if ( !LoadFresnel(fresnelElem, f) ) return false;
        if ( !glosrefl->Attribute("Scale", &scale) ) {
            Severe("GlossyReflection's Scale attribute missing");
            return false;
        }
        if ( !glosrefl->Attribute("Samples", &samples) ) {
            Severe("GlossyReflection's Samples attribute missing");
            return false;
        }
        bsdf = new GlossyReflection(c, f, static_cast<float>(scale), samples);
        return true;
    }

    bool Scene::LoadSpecularTransmission(TiXmlElement *spectran, BSDF *&bsdf)
    {
        Color c(1, 1, 1);
        TiXmlElement *colorElem = (TiXmlElement *) spectran->FirstChild("Color");
        if ( colorElem ) {
            if ( !XmlParseColor(c, colorElem) ) return false;
        }
        double ei, et;
        if ( !spectran->Attribute("EtaI", &ei) ) {
            Severe("SpecularTransmission's EtaI attribute missing");
            return false;
        }
        if ( !spectran->Attribute("EtaT", &et) ) {
            Severe("SpecularTransmission's EtaT attribute missing");
            return false;
        }
        bsdf = new SpecularTransmission(c, (float) ei, (float) et);
        return true;
    }

    bool Scene::LoadGlossyTransmission(TiXmlElement *glostran, BSDF *&bsdf)
    {
        Color c(1, 1, 1);
        double scale;
        int samples;
        TiXmlElement *colorElem = (TiXmlElement *) glostran->FirstChild("Color");
        if ( colorElem ) {
            if ( !XmlParseColor(c, colorElem) ) return false;
        }
        double ei, et;
        if ( !glostran->Attribute("EtaI", &ei) ) {
            Severe("SpecularTransmission's EtaI attribute missing");
            return false;
        }
        if ( !glostran->Attribute("EtaT", &et) ) {
            Severe("SpecularTransmission's EtaT attribute missing");
            return false;
        }
        if ( !glostran->Attribute("Scale", &scale) ) {
            Severe("GlossyReflection's Scale attribute missing");
            return false;
        }
        if ( !glostran->Attribute("Samples", &samples) ) {
            Severe("GlossyReflection's Samples attribute missing");
            return false;
        }
        bsdf = new GlossyTransmission(c, (float) ei, (float) et, (float) scale, samples);
        return true;
    }

    bool Scene::LoadFresnel(TiXmlElement *fresnel, Fresnel *&f)
    {
        string type(fresnel->Attribute("Type"));
        if ( type == "Special" ) {
            double scale;
            if ( !fresnel->Attribute("Scale", &scale) ) {
                Severe("SpecialFresnel's Scale attribute missing");
                return false;
            }
            f = new FresnelSpecial((float) scale);
        } else if ( type == "Dielectric" ) {
            double eta_i = 1, eta_t;
            fresnel->Attribute("EtaI", &eta_i);
            if ( !fresnel->Attribute("EtaT", &eta_t) ) {
                Severe("DielectricFresnel's EtaT attribute missing");
                return false;
            }
            f = new FresnelDielectric((float) eta_i, (float) eta_t);
        } else if ( type == "Conductor" ) {
            double eta, k;
            if ( !fresnel->Attribute("Eta", &eta) ) {
                Severe("ConductorFresnel's Eta attribute missing");
                return false;
            }
            if ( !fresnel->Attribute("K", &k) ) {
                Severe("ConductorFresnel's K attribute missing");
                return false;
            }
            f = new FresnelConductor((float) eta, (float) k);
        } else {
            stringstream ss;
            ss << "Unknown fresnel type: [" << type << "]";
            Severe(ss);
            return false;
        }
        return true;
    }

    bool Scene::LoadSimpleLight(TiXmlElement *light, BSDF *&bsdf)
    {
        Color c(1, 1, 1);
        TiXmlElement *elemColor = (TiXmlElement *) light->FirstChild("Color");
        if ( elemColor ) {
            if ( !XmlParseColor(c, elemColor) ) return false;
        }
        double i, a;
        if ( !light->Attribute("Intensity", &i) ) {
            Severe("SimpleLight's Intensity attribute missing");
            return false;
        }
        if ( !light->Attribute("Alpha", &a) ) {
            Severe("SimpleLight's Alpha attribute missing");
            return false;
        }
        bsdf = new SimpleLight(c, static_cast<float>(i), static_cast<float>(a));
        return true;
    }

    bool Scene::LoadTextures(TiXmlElement *textures)
    {
        stringstream ss;
        if ( gEngineConfig.GetBool("Verbose") ) {
            Info("Loading textures");
        }
        TiXmlElement *texture = (TiXmlElement *) textures->FirstChild("Texture");
        for ( ; texture; texture = texture->NextSiblingElement("Texture") ) {
            const char *name = texture->Attribute("Name");
            if ( !name ) {
                Severe("Texture's Name attribute missing");
                return false;
            }
            const char *file = texture->Attribute("File");
            if ( !file ) {
                ss << "Texture [" << name << "] File attribute missing";
                Severe(ss);
                return false;
            }
            Texture *tex = new Texture(file);
            if ( !tex->IsLoadSuccess() ) {
                ss << "Unable to load texture [" << name << "] [" << file << "]";
                Severe(ss);
                delete tex;
                return false;
            }
            string texName(name);
            mTextures[texName] = tex;
            if ( gEngineConfig.GetBool("Verbose") ) {
                ss << "Texture loaded: [" << name << "]";
                Info(ss);
            }
        }
        return true;
    }
    bool Scene::LoadShapes(TiXmlElement *shapes)
    {
        stringstream ss;
        if ( gEngineConfig.GetBool("Verbose") ) {
            Info("Loading shapes");
        }
        TiXmlElement *shape = (TiXmlElement *) shapes->FirstChild();
        for ( ; shape; shape = shape->NextSiblingElement() ) {
            if ( shape->Type() != TiXmlNode::TINYXML_ELEMENT ) continue;

            if ( !LoadShape(shape) ) return false;
        }
        return true;
    }

    bool Scene::LoadShape(TiXmlElement *shape)
    {
        stringstream ss;

        const char *material = shape->Attribute("Material");
        if ( !material ) {
            Warning("Shape's Material attribute not specified");
            return false;
        }
        string materialName(material);
        Material *m = mMaterials[materialName];
        if ( !m ) {
            ss << "Material not found: [" << materialName << "]";
            Warning(ss);
            return false;
        }

        string shapeType(shape->Value());
        if ( shapeType == "Box" ) {
            if ( !LoadBox(shape, m) ) return false;
        } else if ( shapeType == "Sphere" ) {
            if ( !LoadSphere(shape, m) ) return false;
        } else if ( shapeType == "Cylinder" ) {
            if ( !LoadCylinder(shape, m) ) return false;
        } else if ( shapeType == "TCylinder" ) {
            if ( !LoadTCylinder(shape, m) ) return false;
        } else if ( shapeType == "Plane" ) {
            if ( !LoadPlane(shape, m) ) return false;
        } else if ( shapeType == "TriangleMesh" ) {
            if ( !LoadTriangleMesh(shape, m) ) return false;
        } else {
            ss << "Unknown shape type: " << shapeType;
            Severe(ss);
            return false;
        }
        if ( gEngineConfig.GetBool("Verbose") ) {
            ss << "Added shape: " << shapeType;
            Info(ss);
        }
        return true;
    }

    bool Scene::LoadBox(TiXmlElement *shape, Material *m)
    {
        Vector pos, size;
        TiXmlElement *posElem = (TiXmlElement *) shape->FirstChild("Position");
        if ( !posElem ) {
            Severe("Box's Position attribute missing");
            return false;
        }
        if ( !XmlParseVector(pos, posElem) ) return false;
        TiXmlElement *sizeElem = (TiXmlElement *) shape->FirstChild("Size");
        if ( !sizeElem ) {
            Severe("Box's Size attribute missing");
            return false;
        }
        if ( !XmlParseVector(size, sizeElem) ) return false;
        AddShape(new Box(m, pos, size));
        return true;
    }

    bool Scene::LoadCylinder(TiXmlElement *shape, Material *m)
    {
        Vector pos;
        TiXmlElement *centerElem = (TiXmlElement *) shape->FirstChild("Position");
        if ( !centerElem ) {
            Severe("Cylinder's Position attribute missing");
            return false;
        }
        if ( !XmlParseVector(pos, centerElem) ) return false;
        double radius, halfheight;
        if ( !shape->Attribute("Radius", &radius) ){
            Severe("Cylinder's Radius attribute missing");
            return false;
        }
        if ( !shape->Attribute("HalfHeight", &halfheight) ) {
            Severe("Cylinder's HalfHeight attribute missing");
            return false;
        }
        AddShape(new Cylinder(m, pos, (float) radius, (float) halfheight));
        return true;
    }

    bool Scene::LoadTCylinder(TiXmlElement *shape, Material *m)
    {
        Vector pos;
        TiXmlElement *centerElem = (TiXmlElement *) shape->FirstChild("Position");
        if ( !centerElem ) {
            Severe("TCylinder's Position attribute missing");
            return false;
        }
        if ( !XmlParseVector(pos, centerElem) ) return false;
        double radius, halfheight;
        if ( !shape->Attribute("Radius", &radius) ){
            Severe("TCylinder's Radius attribute missing");
            return false;
        }
        if ( !shape->Attribute("HalfHeight", &halfheight) ) {
            Severe("TCylinder's HalfHeight attribute missing");
            return false;
        }
        AddShape(new TCylinder(m, pos, (float) radius, (float) halfheight));
        return true;
    }

    bool Scene::LoadPlane(TiXmlElement *shape, Material *m)
    {
        Vector n;
        TiXmlElement *nElem = (TiXmlElement *) shape->FirstChild("N");
        if ( !nElem ) {
            Severe("Plane's N attribute missing");
            return false;
        }
        if ( !XmlParseVector(n, nElem) ) return false;
        double d;
        if ( !shape->Attribute("D", &d) ) {
            Severe("Plane's D attribute missing");
            return false;
        }
        AddShape(new Plane(m, n, (float) d));
        return true;
    }

    bool Scene::LoadSphere(TiXmlElement *shape, Material *m)
    {
        Vector pos;
        TiXmlElement *posElem = (TiXmlElement *) shape->FirstChild("Position");
        if ( !posElem ) {
            Severe("Sphere's Position attribute missing");
            return false;
        }
        if ( !XmlParseVector(pos, posElem) ) return false;
        double value;
        const char *v = shape->Attribute("Radius", &value);
        if ( !v ) {
            Severe("Sphere's Radius attribute missing");
            return false;
        }
        AddShape(new Sphere(m, pos, (float) value));
        return true;
    }

    bool Scene::LoadTriangleMeshFromFile(const char *file, Material *m, float scale)
    {
        stringstream ss;
        Lib3dsFile *f = NULL;
        f = lib3ds_file_open(file);
        if ( !f ) {
            ss << "Failed to load 3ds file: " << file;
            Severe(ss);
            return false;
        }
        bool enableSmooth = gEngineConfig.GetBool("EnableSmoothTriangleMesh");
        for ( int i = 0; i < f->nmeshes; i++ ) {
            Lib3dsMesh *mesh = f->meshes[i];
            int nverts = mesh->nvertices;
            int ntris = mesh->nfaces;
            Vector *p = new Vector[nverts];
            Vector *N = NULL;
            float *uv = NULL;
            if ( enableSmooth ) {     
                N = new Vector[nverts];
                for ( int i = 0; i < nverts; i++ ) {
                    N[i] = Vector(0, 0, 0);
                }
            }
            int *v = new int[3 * ntris];
            float pos[3];

            map<int, vector<int> > vertexFaces;
            for ( int n = 0; n < nverts; n++ ) {
                lib3ds_vector_copy(pos, mesh->vertices[n]);
                p[n] = Vector(-pos[0], pos[1], pos[2]) * scale;
            }
            if ( mesh->texcos ) {
                uv = new float[nverts * 2];
                for ( int n = 0; n < nverts; n++ ) {
                    uv[n*2] = mesh->texcos[n][0];
                    uv[n*2+1] = mesh->texcos[n][1];
                }
            } else {
                ss << "Mesh [" << file << "] contains no UVs";
                Info(ss);
            }
            for ( int n = 0; n < ntris; n++ ) {
                int n0 = n*3, n1 = n0+1, n2 = n1+1;
                v[n0] = mesh->faces[n].index[0];
                v[n1] = mesh->faces[n].index[2];
                v[n2] = mesh->faces[n].index[1];
                if ( enableSmooth ) {
                    const Vector &v0 = p[v[n0]];
                    const Vector &v1 = p[v[n1]];
                    const Vector &v2 = p[v[n2]];
                    Vector norm = (v1 - v0).Cross(v2 - v0);
                    if ( norm.LengthSquared() == 0 ) {
                        Warning("Triangle with invalid normal");
                        continue;
                    }
                    N[v[n0]] += norm;
                    N[v[n1]] += norm;
                    N[v[n2]] += norm;
                }
            }
            if ( enableSmooth ) {
                for ( int i = 0; i < nverts; i++ ) {
                    N[i].Normalize();
                }
            }

            TriangleMesh *trimesh = new TriangleMesh(m, ntris, nverts, v, p, enableSmooth ? N : NULL, uv);
            AddShape(trimesh);
            for ( int i = 0; i < ntris; i++ ) {
                AddShape(new Triangle(m, trimesh, i));
            }
            if ( p ) delete [] p;
            if ( v ) delete [] v;
            if ( uv ) delete [] uv;
            if ( enableSmooth ) delete [] N;
        }
        if ( gEngineConfig.GetBool("Verbose") ) {
            ss << "Mesh loaded: [" << file << "]";
            Info(ss);
        }
        return true;
    }

    bool Scene::LoadTriangleMesh(TiXmlElement *shape, Material *m)
    {
        stringstream ss;
        const char *file = shape->Attribute("File");
        double d;
        float scale = 1.f;
        if ( shape->Attribute("Scale", &d) ) scale = (float) d;
        if ( file ) {
            return LoadTriangleMeshFromFile(file, m, scale);
        }
        TiXmlElement *verticesElem = (TiXmlElement *) shape->FirstChild("Vertices");
        TiXmlElement *trianglesElem = (TiXmlElement *) shape->FirstChild("Triangles");
        if ( !verticesElem ) {
            Severe("TriangleMesh's Vertices attribute missing");
            return false;
        }
        if ( !trianglesElem ) {
            Severe("TriangleMesh's Triangles attribute missing");
        }
        vector<Vector> vertices;
        int nverts = 0;
        TiXmlElement *vertex = (TiXmlElement *) verticesElem->FirstChild("Vertex");
        for ( ; vertex; vertex = vertex->NextSiblingElement("Vertex") ) {
            Vector v;
            if ( !XmlParseVector(v, vertex) ) return false;
            v *= scale;
            vertices.push_back(v);
            nverts++;
        }
        TiXmlElement *triangle = (TiXmlElement *) trianglesElem->FirstChild("Triangle");
        vector<int> triangles;
        vector<Material *> mtrls;
        int ntris = 0;
        for ( ; triangle; triangle = triangle->NextSiblingElement("Triangle") ) {
            int v0, v1, v2;
            if ( !triangle->Attribute("V0", &v0) ) {
                Severe("Triangle's V0 attribute missing");
                return false;
            }
            if ( !triangle->Attribute("V1", &v1) ) {
                Severe("Triangle's V1 attribute missing");
                return false;
            }
            if ( !triangle->Attribute("V2", &v2) ) {
                Severe("Triangle's V2 attribute missing");
                return false;
            }
            triangles.push_back(v0);
            triangles.push_back(v1);
            triangles.push_back(v2);

            const char *material = triangle->Attribute("Material");
            Material *mtrl;
            if ( !material ) {
                mtrl = m;
            } else {
                string materialName(material);
                mtrl = mMaterials[materialName];
                if ( !mtrl ) {
                    ss << "Material not found: [" << materialName << "]";
                    Warning(ss);
                    return false;
                }
            }
            mtrls.push_back(mtrl);
            ntris++;
        }

        Vector *p = new Vector[nverts];
        int *v = new int[3 * ntris];
        
        for ( int i = 0; i < nverts; i++ ) {
            p[i] = vertices[i];
        }
        for ( int i = 0; i < ntris; i++ ) {
            v[i*3] = triangles[i*3];
            v[i*3+1] = triangles[i*3+1];
            v[i*3+2] = triangles[i*3+2];
        }
        TriangleMesh *mesh = new TriangleMesh(m, ntris, nverts, v, p, NULL, NULL);
        AddShape(mesh);
        for ( int i = 0; i < ntris; i++ ) {
            AddShape(new Triangle(mtrls[i], mesh, i));
        }
        delete [] p;
        delete [] v;
        return true;
    }

    bool Scene::LoadLights(TiXmlElement *lights)
    {
        stringstream ss;
        if ( gEngineConfig.GetBool("Verbose") ) {
            Info("Loading lights");
        }
        TiXmlElement *light = (TiXmlElement *) lights->FirstChild();
        for ( ; light; light = light->NextSiblingElement() ) {
            if ( light->Type() != TiXmlNode::TINYXML_ELEMENT ) continue;
            if ( !LoadLight(light) ) return false;
        }
        return true;
    }

    bool Scene::LoadLight(TiXmlElement *light)
    {
        stringstream ss;
        Color color;
        TiXmlElement *elemColor = (TiXmlElement *) light->FirstChild("Color");
        if ( !elemColor ) {
            Severe("Light's Color attribute missing");
            return false;
        }
        if ( !XmlParseColor(color, elemColor) ) return false;
        double intensity;
        if ( !light->Attribute("Intensity", &intensity) ) {
            Severe("Light's Intensity attribute missing");
            return false;
        }
        double alpha = 0;
        light->Attribute("Alpha", &alpha);
        string lightType(light->Value());
        if ( lightType == "PointLight" ) {
            if ( !LoadPointLight(light, color, (float) intensity, (float) alpha) ) return false;
        } else if ( lightType == "AreaLight" ) {
            if ( !LoadAreaLight(light, color, (float) intensity, (float) alpha)) return false;
        } else if ( lightType == "SpotLight" ) {
            if ( !LoadSpotLight(light, color, (float) intensity, (float) alpha)) return false;
        } else {
            ss << "Unknown light type: " << lightType;
            Severe(ss);
            return false;
        }
        if ( gEngineConfig.GetBool("Verbose") ) {
            ss << "Added light: " << lightType;
            Info(ss);
        }
        return true;
    }

    bool Scene::LoadPointLight(TiXmlElement *light, const Vector &color, float intensity, float alpha)
    {
        Vector pos;
        TiXmlElement *elemPos = light->FirstChildElement("Position");
        if ( !elemPos ) {
            Warning("PointLight's Position attribute missing");
            return false;
        }
        if ( !XmlParseVector(pos, elemPos) ) return false;
        AddLight(new PointLight(pos, color, intensity, alpha));
        return true;
    }

    bool Scene::LoadAreaLight(TiXmlElement *light, const Vector &color, float intensity, float alpha)
    {
        Vector pos, size;
        TiXmlElement *elemPos = light->FirstChildElement("Position");
        if ( !elemPos ) {
            Severe("AreaLight's Position attribute missing");
            return false;
        }
        if ( !XmlParseVector(pos, elemPos) ) return false;
        TiXmlElement *elemSize = light->FirstChildElement("Size");
        if ( !elemSize ) {
            Severe("AreaLight's Size attribute missing");
            return false;
        }
        if ( !XmlParseVector(size, elemSize) ) return false;
        AddLight(new AreaLight(pos, size, color, intensity, alpha));
        return true;
    }

    bool Scene::LoadSpotLight(TiXmlElement *light, const Vector &color, float intensity, float alpha)
    {
        Vector pos, dir;
        TiXmlElement *elemPos = light->FirstChildElement("Position");
        if ( !elemPos ) {
            Severe("SpotLight's Position attribute missing");
            return false;
        }
        if ( !XmlParseVector(pos, elemPos) ) return false;
        TiXmlElement *elemDir = light->FirstChildElement("Dir");
        if ( !elemDir ) {
            Severe("SpotLight's Dir attribute missing");
            return false;
        }
        if ( !XmlParseVector(dir, elemDir) ) return false;
        double a0, a1;
        if ( !light->Attribute("A0", &a0) ) {
            Severe("SpotLight's A0 attribute missing");
            return false;
        }
        if ( !light->Attribute("A1", &a1) ) {
            Severe("SpotLight's A1 attribute missing");
            return false;
        }
        AddLight(new SpotLight(pos, dir, (float) a0, (float) a1, color, intensity, alpha));
        return true;
    }
} // namespace LyraeFX