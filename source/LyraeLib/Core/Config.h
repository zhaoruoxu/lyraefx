
#if defined(_MSC_VER)
#pragma once
#endif

#ifndef LYRAE_CORE_CONFIG_H
#define LYRAE_CORE_CONFIG_H

#include "LyraeFX.h"
#include "Geometry.h"

namespace LyraeFX
{
    class Config
    {
    public:
        Config() {}

        void AddAttribute(string name, float f)
        {
            floatAttrs[name] = f;
        }
        void AddAttribute(string name, int32_t i)
        {
            int32Attrs[name] = i;
        }
        void AddAttribute(string name, const Vector &v)
        {
            vectorAttrs[name] = v;
        }
        void AddAttribute(string name, bool b)
        {
            boolAttrs[name] = b;
        }
        float GetFloat(string name)
        {
            _ASSERT( floatAttrs.find(name) != floatAttrs.end() );
            return floatAttrs[name];
        }
        int32_t GetInt32(string name)
        {
            _ASSERT( int32Attrs.find(name) != int32Attrs.end() );
            return int32Attrs[name];
        }
        Vector GetVector(string name)
        {
            _ASSERT( vectorAttrs.find(name) != vectorAttrs.end() );
            Vector v = vectorAttrs[name];
            return v;
        }
        bool GetBool(string name)
        {
            _ASSERT( boolAttrs.find(name) != boolAttrs.end() );
            return boolAttrs[name];
        }

        map<string, float> floatAttrs;
        map<string, int32_t> int32Attrs;
        map<string, Vector> vectorAttrs;
        map<string, bool> boolAttrs;
    }; // class Config

} // namespace LyraeFX

#endif // LYRAE_CORE_CONFIG_H