#include "EngineConfig.h"
#include "Engine.h"
#include "Error.h"
#include "../TinyXml/tinyxml.h"

namespace LyraeFX
{
    EngineConfig gEngineConfig;

    void EngineConfig::LoadDefault()
    {
        TiXmlDocument doc(DefaultConfigFileName.c_str());
        if ( !doc.LoadFile() ) {
            Warning("Cannot load default engine configuration");
            return;
        }
        TiXmlHandle docHandle(&doc);
        TiXmlElement *rootElem = doc.FirstChild("LyraeDefault")->FirstChild("Config")->ToElement();
        if ( !rootElem ) {
            Warning("Invalid default engine configuration");
            return;
        }
        if ( !XmlParseConfig(gEngineConfig, rootElem) ) {
            Warning("Loading default engine configuration failed");
        }
    }
}


