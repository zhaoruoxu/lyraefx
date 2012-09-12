#include "Task.h"
#include "LyraeFX.h"
#include "Engine.h"
#include "Error.h"

namespace LyraeFX
{
    int32_t Task::taskCount = 0;

    void RenderTask::Run()
    {
        stringstream ss;
        bool renderOkay = gEngine.Render(width0, width1, height0, height1);
        if ( !renderOkay ) {
            ss << "Task [" << taskID << "] failed.";
            Warning(ss);
        }
    }

    Task *TaskManager::ExtractTask()
    {
        Task *result = NULL;
        EnterCriticalSection(&cs);
        if ( tasks.size() > 0 ) {
            result = tasks.front();
            tasks.pop_front();
        }
        LeaveCriticalSection(&cs);
        return result;
    }

    TaskManager::~TaskManager()
    {
        Reset();
        DeleteCriticalSection(&cs);
    }

    void TaskManager::Reset()
    {
        EnterCriticalSection(&cs);
        if ( tasks.size() > 0 ) {
            for ( uint32_t i = 0; i < tasks.size(); i++ ) {
                delete tasks[i];
            }
            tasks.clear();
        }
        LeaveCriticalSection(&cs);
    }

    void TaskManager::InitTasks()
    {
        CreateRenderTasks(gEngineConfig.GetInt32("RenderTileSize"));
    }

    void TaskManager::CreateRenderTasks(int32_t tileSize)
    {

        int32_t width = gEngineConfig.GetInt32("TargetWidth");
        int32_t height = gEngineConfig.GetInt32("TargetHeight");
        int xNum = width / tileSize;
        int yNum = height / tileSize;
        if ( width > tileSize * xNum ) xNum++;
        if ( height > tileSize * yNum ) yNum++;

        int taskNum = 0;
        int offsetMax = (min(xNum, yNum) + 1) / 2;
        for ( int y = 0; y < yNum; y++ ) {
            if ( y % 2 == 0 ) {
                for ( int x = xNum - 1; x >= 0; x-- ) {
                    int32_t x0 = x * tileSize;
                    int32_t y0 = y * tileSize;
                    int32_t x1 = min(width, x0 + tileSize);
                    int32_t y1 = min(height, y0 + tileSize);
                    RenderTask *task = new RenderTask(x0, x1, y0, y1);
                    tasks.push_back(task);
                }
            } else {
                for ( int x = 0; x < xNum; x++ ) {
                    int32_t x0 = x * tileSize;
                    int32_t y0 = y * tileSize;
                    int32_t x1 = min(width, x0 + tileSize);
                    int32_t y1 = min(height, y0 + tileSize);
                    RenderTask *task = new RenderTask(x0, x1, y0, y1);
                    tasks.push_back(task);
                }
            }

        }

#undef ADD_TASK
    }
}