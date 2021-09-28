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
    explicit ThreadPool(int num) :
        semaphores{ false }
    {
        for (int i = 0; i < num; i++)
        {
            semaphores[i] = true;
            threads.emplace_back([=] {  while (true) {
                static Thread::Semaphore *semaphore = semaphores + i;
                *semaphores = true;
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
                    *semaphore = false;
                    task = std::move(tasks.front());
                    tasks.pop();
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
