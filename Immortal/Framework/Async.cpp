#include "Async.h"

namespace Immortal
{

std::unique_ptr<ThreadPool> Async::threadPool{ nullptr };

ThreadPool::ThreadPool(uint32_t num)
{
    for (int i = 0; i < num; i++)
    {
        semaphores[i] = Thread::State::Idle;
        threads.emplace_back([=, this]() -> void {
            while (true)
            {
                static Thread::Semaphore *semaphore = semaphores + i;
                Task task;
                {
                    std::unique_lock<std::mutex> lock{ mutex };
                    condition.wait(lock, [=, this] { 
                            return stopping || !tasks.empty();
                        });

                    if (stopping)
                    {
                        break;
                    }

                    *semaphore = Thread::State::Block;
                    task = std::move(tasks.front());
                    tasks.pop();
                }
                task();
                *semaphore = Thread::State::Idle;
            }
            });
    }
}

ThreadPool::~ThreadPool()
{
    {
        std::unique_lock<std::mutex> lock{ mutex };
        stopping = true;
    }
    condition.notify_all();

    for (auto &thread : threads)
    {
        thread.join();
    }
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
