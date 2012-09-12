#if defined(_MSC_VER)
#pragma once
#endif

#ifndef LYRAE_CONFIG_ENGINECONFIG_H
#define LYRAE_CONFIG_ENGINECONFIG_H

#include "../Core/LyraeFX.h"
#include "../Core/Config.h"
#include "../Core/Geometry.h"

namespace LyraeFX
{

    class EngineConfig : public Config
    {
    public:
        EngineConfig() { LoadDefault(); }

    protected:
        void LoadDefault();
    }; // class EngineConfig

    extern EngineConfig gEngineConfig;
} // namespace LyraeFX

#endif // LYRAE_CONFIG_ENGINECONFIG_H