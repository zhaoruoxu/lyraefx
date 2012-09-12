
#include "Box.h"
#include "../Config/EngineConfig.h"

namespace LyraeFX
{

    Vector Box::GetNormal(const Vector &pos)
    {
        float dist[6];

        dist[0] = (float) fabs(pos.x - mBox.mPos.x);
        dist[1] = (float) fabs(pos.x - mBox.mSize.x - mBox.mPos.x);
        dist[2] = (float) fabs(pos.y - mBox.mPos.y);
        dist[3] = (float) fabs(pos.y - mBox.mSize.y - mBox.mPos.y);
        dist[4] = (float) fabs(pos.z - mBox.mPos.z);
        dist[5] = (float) fabs(pos.z - mBox.mSize.z - mBox.mPos.z);
        int best = 0;
        float bestDist = dist[0];
        for ( int i = 0; i < 6; i++ ) if ( dist[i] < bestDist )
        {
            bestDist = dist[i];
            best = i;
        }
        switch( best )
        {
        case 0:
            return Vector(-1, 0, 0);
        case 1:
            return Vector(1, 0, 0);
        case 2:
            return Vector(0, -1, 0);
        case 3:
            return Vector(0, 1, 0);
        case 4:
            return Vector(0, 0, -1);
        case 5:
            return Vector(0, 0, 1);
        default:
            assert(0);
        }

        return Vector(0, 0, 0);
    }

    Color Box::GetColor(const Vector &pos)
    {
        if ( !mMaterial->mTexture )
            return mMaterial->mColor;

        float e = EPSILON * 4.f;
        float u = 0, v = 0;
        if ( abs( pos.x - mBox.mPos.x ) < e ||
             abs( pos.x - mBox.mPos.x - mBox.mSize.x ) < e )
        {
            u = ( pos.y - mBox.mPos.y ) / mBox.mSize.y;
            v = ( pos.z - mBox.mPos.z ) / mBox.mSize.z;
        }
        else if ( abs( pos.y - mBox.mPos.y ) < e ||
                  abs( pos.y - mBox.mPos.y - mBox.mSize.y ) < e )
        {
            u = ( pos.z - mBox.mPos.z ) / mBox.mSize.z;
            v = ( pos.x - mBox.mPos.x ) / mBox.mSize.x;
        }
        else if ( abs( pos.z - mBox.mPos.z ) < e ||
                  abs( pos.z - mBox.mPos.z - mBox.mSize.z ) < e )
        {
            u = ( pos.x - mBox.mPos.x ) / mBox.mSize.x;
            v = ( pos.y - mBox.mPos.y ) / mBox.mSize.y;
        }
        else
        {
            assert(0);
        }
        float uu = fabsf(u) * mMaterial->mUScale;
        float vv = fabsf(v) * mMaterial->mVScale;
        return mMaterial->mTexture->GetTexel(uu, vv) * mMaterial->mColor;
    }

} // namespace LyraeFX