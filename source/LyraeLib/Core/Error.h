#if defined(_MSC_VER)
#pragma once
#endif

#ifndef LYRAE_CORE_ERROR_H
#define LYRAE_CORE_ERROR_H

#include "LyraeFX.h"
#include "Engine.h"

namespace LyraeFX
{
    inline void Info(string s)
    {
        gEngine.AddInfo(s);
    }
    inline void Info(stringstream &ss)
    {
        gEngine.AddInfo(ss);
    }
    inline void Warning(string s)
    {
        gEngine.AddWarning(s);
    }
    inline void Warning(stringstream &ss)
    {
        gEngine.AddWarning(ss);
    }
    inline void Severe(string s)
    {
        gEngine.AddSevere(s);
    }
    inline void Severe(stringstream &ss)
    {
        gEngine.AddSevere(ss);
    }
}

#endif // LYRAE_CORE_ERROR_H