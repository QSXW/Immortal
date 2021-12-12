#include "impch.h"
#include "ThreadManager.h"

namespace Immortal
{

std::unique_ptr<ThreadPool> Async::threadPool{ nullptr };

void Async::Setup()
{
    int count = std::thread::hardware_concurrency();
    
    Profiler p;
    LOG::INFO("Creating {0} Thread(s)", count);

    threadPool.reset(new ThreadPool{ count });
}

#pragma sl_disable_optimizations
void ThreadPool::Join()
{
    bool tasking{ true };
    while (!tasks.empty()) { }

    while (1)
    {
        tasking = false;
        for (int i = 0; i < threads.size(); i++)
        {
            if (semaphores[i] == Thread::State::Block)
            {
                tasking = true;
                break;
            }
        }
        if (!tasking)
        {
            break;
        }
    }
}
#pragma sl_enable_optimizations

}
