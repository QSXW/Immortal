#pragma once

#include "Core.h"
#include "Log.h"
#include "Interface/IObject.h"

#include <thread>
#include <queue>
#include <future>
#include <functional>
#include <atomic>

namespace Immortal
{

class Thread
{
public:
	SL_SWAPPABLE(Thread)

    enum class State : uint32_t
    {
        Block = BIT(0),
        Idle  = BIT(1)
    };

    using Semaphore = State;

    static uint32_t Id()
    {
#ifndef __GNUC__
        return GetCurrentThreadId();
#else
		return (uint32_t)pthread_self();
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
        handle = std::jthread{ stub };
    }

    void Join()
    {
        if (handle.joinable())
        {
			handle.join();
        }
    }

    void SetDescription(const std::string &name)
    {
#ifdef _WIN32
        SetThreadDescription(handle.native_handle(), std::wstring(name.begin(), name.end()).c_str());
#endif
    }

    void SetDebugDescription(const std::string &name)
    {
#ifdef _DEBUG
        SetDescription(name);
#else
		(void) name;
#endif
    }

    void Swap(Thread &other)
    {
		std::swap(stub,   other.stub  );
		std::swap(handle, other.handle);
    }

protected:
    std::function<void()> stub;

    std::jthread handle;
};

using Task = std::function<void()>;

class ThreadPool
{
public:
    ThreadPool(uint32_t numThreads);

    ~ThreadPool();

    void Join();

    void RemoveTasks();

    const std::atomic<uint32_t> &TaskSize() const;

public:
    template <class T>
    auto Enqueue(T task)->std::future<decltype(task())>
    {
		tasked = true;
		taskRef++;
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

protected:
    std::vector<std::thread> threads;
    
    std::atomic<uint32_t> taskRef;

    std::atomic<bool> tasked;

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
