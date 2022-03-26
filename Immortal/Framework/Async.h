#pragma once

#include "Core.h"
#include "Log.h"

#include <thread>
#include <queue>
#include <future>
#include <functional>

namespace Immortal
{

class Thread
{
public:
    enum class State : uint32_t
    {
        Block = BIT(0),
        Idle  = BIT(1)
    };

    using Semaphore = State;

    static int Self()
    {
#ifndef __GNUC__
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
    ThreadPool(uint32_t num);

    ~ThreadPool();

    void Join();

public:
    template <class T>
    auto Enqueue(T task)->std::future<decltype(task())>
    {
        auto wrapper = std::make_shared<std::packaged_task<decltype(task())()>>(std::move(task));
        {
            std::unique_lock<std::mutex> lock{ mutex };
            tasks.emplace([=]() -> void {
                (*wrapper)();
            });
        }

        condition.notify_one();
        return wrapper->get_future();
    }

private:
    std::vector<std::thread> threads;

    Thread::Semaphore semaphores[256];

    std::condition_variable condition;

    std::queue<Task> tasks;

    std::mutex mutex;

    bool stopping{ false };
};

class Async
{
public:
    static void Setup()
    {
        Profiler p{ "Initializing Asynchronous Library" };
        threadPool.reset(new ThreadPool{ std::thread::hardware_concurrency() });
    }

    template <class T>
    static auto Execute(T &&task)
    {
        return threadPool->Enqueue(task);
    }

    static void Wait()
    {
        threadPool->Join();
    }

public:
    static std::unique_ptr<ThreadPool> threadPool;
};

}
