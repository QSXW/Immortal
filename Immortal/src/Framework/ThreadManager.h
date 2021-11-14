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
    explicit ThreadPool(int num)
    {
        for (int i = 0; i < num; i++)
        {
            semaphores[i] = Thread::State::Idle;
            threads.emplace_back([=]() -> void {  while (true) {
                static Thread::Semaphore *semaphore = semaphores + i;
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
                    *semaphore = Thread::State::Block;
                    task = std::move(tasks.front());
                    tasks.pop();
                }
                task();
                *semaphore = Thread::State::Idle;
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
            tasks.emplace([=]() -> void {
                (*wrapper)();
            });
        }

        condition.notify_one();
        return wrapper->get_future();
    }

    void Join();

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
