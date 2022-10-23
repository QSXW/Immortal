#include "Async.h"

namespace Immortal
{

std::unique_ptr<ThreadPool> Async::threadPool{ nullptr };

ThreadPool::ThreadPool(uint32_t numThreads)
{
    threads.reserve(numThreads);
    for (int i = 0; i < numThreads; i++)
    {
        threads.emplace_back([=, this]() -> void {
            while (true)
            {
                Anonymous semaphore = nullptr;
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

                    auto &refTask = tasks.front();
                    task = refTask;
                    semaphore = &refTask;
                    tasks.pop();
                }
                task();
                
                std::lock_guard lock{mutex};
                semaphores.erase(semaphore);
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
    while (!tasks.empty()) {}
    while (!semaphores.empty()) {}
}
#pragma sl_enable_optimizations

}
