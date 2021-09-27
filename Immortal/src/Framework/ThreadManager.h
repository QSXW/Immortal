#pragma once

#include "ImmortalCore.h"

#include <thread>
#include <queue>
#include <future>
#include <functional>

namespace Immortal
{

class Thread
{
public:
    using Semaphore = bool;
    static int Self()
    {
#ifdef WINDOWS
        return GetCurrentThreadId();
#else 
        return (int)pthread_self();
#endif
    }
};

using Task = std::function<void()>;

class ThreadPool
{
public:
    explicit ThreadPool(int num)
    {
        semaphores.reset(new Thread::Semaphore[num]);
        for (int i = 0; i < num; i++)
        {
            semaphores.get()[i] = true;
            threads.emplace_back([=] {  while (true) {
                static Thread::Semaphore *semaphore = semaphores.get() + i;
                Task task;
                {
                    std::unique_lock<std::mutex> lock{ mutex };
                    condition.wait(lock, [=] { return stopping || !tasks.empty(); });
                    if (stopping)
                    {
                        LOG::INFO("Stopping Thread -> \tid => {0}\thandle => {1}",
                                  threads[i].get_id(), threads[i].native_handle());
                        break;
                    }
                    task = std::move(tasks.front());
                    tasks.pop();
                    *semaphore = false;
                }
                task();
                *semaphore = true;
            }});
            LOG::INFO("\tid => {0}\thandle => {1}", threads[i].get_id(), threads[i].native_handle());
        }
    }

    ~ThreadPool()
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

    template <class T>
    auto Enqueue(T task)->std::future<decltype(task())>
    {
        auto wrapper = std::make_shared<std::packaged_task<decltype(task())()>>(std::move(task));
        {
            std::unique_lock<std::mutex> lock{ mutex };
            tasks.emplace([=]{
                (*wrapper)();
            });
        }

        condition.notify_one();
        return wrapper->get_future();
    }

    void Join()
    {
        while (!tasks.empty()) {}

        struct DebugInfo
        {
            bool data[16];
        } *debugInfo;

        debugInfo = (DebugInfo *)semaphores.get();

        bool tasking{ true };

        while (tasking)
        {
            tasking = false;
            for (int i = 0; i < threads.size(); i++)
            {
                if (!semaphores.get()[i])
                {
                    tasking = true;
                    break;
                }
            }
        }
    }

private:
    std::vector<std::thread> threads;

    std::unique_ptr<Thread::Semaphore> semaphores;

    std::condition_variable condition;

    std::queue<Task> tasks;

    std::mutex mutex;

    bool stopping{ false };
};

class Async
{
public:
    static void INIT();

    template <class T>
    static auto Execute(T &&task)
    {
        return threadPool->Enqueue(task);
    }

    static void Join()
    {
        threadPool->Join();
    }

public:
    static std::unique_ptr<ThreadPool> threadPool;
};

}
