#if defined(_MSC_VER)
#pragma once
#endif

#ifndef LYRAE_CORE_TASK_H
#define LYRAE_CORE_TASK_H

#include "LyraeFX.h"

namespace LyraeFX
{
    class Task
    {
    public:
        Task() : taskID(taskCount++) {}

        static int32_t taskCount;
        const int32_t taskID;
        virtual void Run() = 0;
    };

    class RenderTask : public Task
    {
    public:
        RenderTask(int32_t w0, int32_t w1, int32_t h0, int32_t h1)
            :width0(w0), width1(w1), height0(h0), height1(h1)
        {
        }

        void Run();

        int32_t width0, width1;
        int32_t height0, height1;
    };

    class TaskManager
    {
    public:
        TaskManager()
        {
            InitializeCriticalSection(&cs);
        }
        ~TaskManager();

        Task *ExtractTask();
        void Reset();
        void InitTasks();
        void CreateRenderTasks(int32_t tileSize);

        CRITICAL_SECTION cs;
        deque<Task *> tasks;
    };
}


#endif // LYRAE_CORE_TASK_H