#include "LyraeFX.h"
#include "Parallel.h"
#include "Engine.h"
#include "../Config/EngineConfig.h"
#include "Error.h"
#include "Task.h"

namespace LyraeFX
{
    unsigned int __stdcall RenderWorkThread(LPVOID param)
    {
        RenderThreadParam *p = (RenderThreadParam *) param;

        Task *task = NULL;
        while ( true ) {
            task = gEngine.taskManager.ExtractTask();
            if ( !task ) break;
            task->Run();
            delete task;
        }
        return 0;
    }

    unsigned int __stdcall PhotonThread(LPVOID param)
    {
        RenderThreadParam *p = (RenderThreadParam *) param;

        gEngine.ThreadBuildPhotonMap(p->threadID);
        return 0;
    }

}