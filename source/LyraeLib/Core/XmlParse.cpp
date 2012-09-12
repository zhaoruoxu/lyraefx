#include "XmlParse.h"
#include "Engine.h"
#include "Error.h"

namespace LyraeFX
{
    bool XmlParseColor(Color &c, TiXmlElement *color)
    {
        if ( color == NULL ) return false;
        double r, g, b;
        const char *pAttrR = color->Attribute("R", &r);
        const char *pAttrG = color->Attribute("G", &g);
        const char *pAttrB = color->Attribute("B", &b);
        if ( !pAttrR || !pAttrG || !pAttrB ) {
            Severe("Invalid color value");
            return false;
        }
        c.x = (float) r;
        c.y = (float) g;
        c.z = (float) b;
        return true;
    }
    bool XmlParseVector(Vector &v, TiXmlElement *vec)
    {
        if ( vec == NULL ) return false;
        double x, y, z;
        const char *pAttrX = vec->Attribute("X", &x);
        const char *pAttrY = vec->Attribute("Y", &y);
        const char *pAttrZ = vec->Attribute("Z", &z);
        if ( !pAttrX || !pAttrY || !pAttrZ ) {
            Severe("Invalid vector value");
            return false;
        }
        v.x = (float) x;
        v.y = (float) y;
        v.z = (float) z;
        return true;
    }
    bool XmlParseConfig(Config &c, TiXmlElement *conf)
    {
        if ( conf == NULL ) return false;
        stringstream ss;
        TiXmlElement *config = conf->FirstChildElement();
        for ( ; config; config = config->NextSiblingElement() ) {
            if ( config->Type() != TiXmlNode::TINYXML_ELEMENT ) continue;
            string configType(config->Value());
            const char *name = config->Attribute("Name");
            if ( !name ) {
                Severe("Config Name attribute missing");
                return false;
            }
            string configName(name);
            if ( configType == "Float" ) {
                double value;
                if ( !config->Attribute("Value", &value) ) {
                    ss << "Config [" << configName << "] missing value";
                    Severe(ss);
                    return false;
                }
                c.AddAttribute(configName, (float) value);
            } else if ( configType == "Int32" ) {
                int i;
                if ( !config->Attribute("Value", &i) ) {
                    ss << "Config [" << configName << "] missing value";
                    Severe(ss);
                    return false;
                }
                c.AddAttribute(configName, i);
            } else if ( configType == "Vector" ) {
                Vector vec;
                TiXmlElement *elemVec = config->FirstChildElement("Value");
                if ( !elemVec ) {
                    ss << "Config [" << configName << "] missing value";
                    Severe(ss);
                    return false;
                }
                if ( !XmlParseVector(vec, elemVec) ) return false;
                c.AddAttribute(configName, vec);
            } else if ( configType == "Bool" ) {
                int b;
                if ( !config->Attribute("Value", &b) ) {
                    ss << "Config [" << configName << "] missing value";
                    Severe(ss);
                    return false;
                }
                c.AddAttribute(configName, b != 0);
            } else {
                ss << "Unknown config type: [" << configType << "]";
                Severe(ss);
                return false;
            }
        }
        return true;
    }
}