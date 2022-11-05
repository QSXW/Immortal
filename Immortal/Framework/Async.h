#pragma once

#include "Core.h"
#include "Log.h"

#include <thread>
#include <queue>
#include <future>
#include <functional>
#include <Interface/IObject.h>

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

    static long Id()
    {
#ifndef __GNUC__
        return GetCurrentThreadId();
#else
        return (long)pthread_self();
#endif
    }

public:
    Thread() :
        stub{},
        handle{}
    {

    }

    template <class T>
    Thread(T task)
    {
        auto wrapper = std::make_shared<std::packaged_task<decltype(task())()>>(std::move(task));
        stub = [=] {
            (*wrapper)();
        };
    }

    ~Thread()
    {
		Join();
    }

    void Start()
    {
        handle = new std::jthread{ stub };
    }

    void Join()
    {
        if (handle->joinable())
        {
			handle->join();
        }
    }

    std::function<void()> stub;

    URef<std::jthread> handle;
};

using Task = std::function<void()>;

class ThreadPool
{
public:
    ThreadPool(uint32_t numThreads);

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
            semaphores.insert((Anonymous) &tasks.back());
        }

        condition.notify_one();
        return wrapper->get_future();
    }

private:
    std::vector<std::thread> threads;

    std::set<Anonymous> semaphores;

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
        LOG::INFO("Initializing Asynchronous Library");
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

    static void Release()
    {
        Wait();
        threadPool.reset();
    }

public:
    static std::unique_ptr<ThreadPool> threadPool;
};

}
