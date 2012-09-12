#include "LyraeFX.h"
#include "Geometry.h"
#include "../TinyXml/tinyxml.h"

namespace LyraeFX
{
    bool XmlParseColor(Color &c, TiXmlElement *color);
    bool XmlParseVector(Vector &v, TiXmlElement *vec);
    bool XmlParseConfig(Config &c, TiXmlElement *conf);
}