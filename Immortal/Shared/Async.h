#pragma once

#include "Core.h"

#include <thread>
#include <queue>
#include <future>
#include <functional>
#include <atomic>
#include <concurrentqueue.h>
#include <lightweightsemaphore.h>

#ifdef __APPLE__
namespace std
{
using jthread = thread;
}
#endif

namespace Immortal
{

//template <class T>
//class ConcurrentQueue : public moodycamel::ConcurrentQueue<T>
//{
//
//};

#include <algorithm>
#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>
#include <utility>

template <typename T>
class ConcurrentQueue
{
private:
	std::queue<T> queue_;
	mutable std::mutex mutex_;
	std::condition_variable cond_;

public:
	ConcurrentQueue() = default;

	ConcurrentQueue(ConcurrentQueue &&other) noexcept
	{
		std::lock_guard<std::mutex> lock(other.mutex_);
		queue_ = std::move(other.queue_);
	}

	ConcurrentQueue &operator=(ConcurrentQueue &&other) noexcept
	{
		if (this != &other)
		{
			std::scoped_lock lock(mutex_, other.mutex_);
			queue_ = std::move(other.queue_);
		}
		return *this;
	}

	ConcurrentQueue(const ConcurrentQueue &) = delete;
	ConcurrentQueue &operator=(const ConcurrentQueue &) = delete;

	void enqueue(const T &item)
	{
		std::lock_guard<std::mutex> lock(mutex_);
		queue_.push(item);
		cond_.notify_one();
	}

	bool enqueue(T &&item)
	{
		std::lock_guard<std::mutex> lock(mutex_);
		queue_.push(std::move(item));
		cond_.notify_one();
		return true;
	}

	template <typename... Args>
	void emplace(Args &&...args)
	{
		std::lock_guard<std::mutex> lock(mutex_);
		queue_.emplace(std::forward<Args>(args)...);
		cond_.notify_one();
	}

	bool try_dequeue(T &item)
	{
		std::lock_guard<std::mutex> lock(mutex_);
		if (queue_.empty())
		{
			return false;
		}
		item = std::move(queue_.front());
		queue_.pop();
		return true;
	}

	template <typename Rep, typename Period>
	bool wait_for_pop(T &item, const std::chrono::duration<Rep, Period> &timeout)
	{
		std::unique_lock<std::mutex> lock(mutex_);
		if (!cond_.wait_for(lock, timeout, [this] { return !queue_.empty(); }))
		{
			return false;
		}
		item = std::move(queue_.front());
		queue_.pop();
		return true;
	}

	T wait_and_pop()
	{
		std::unique_lock<std::mutex> lock(mutex_);
		cond_.wait(lock, [this] { return !queue_.empty(); });
		T item = std::move(queue_.front());
		queue_.pop();
		return item;
	}

	void swap(ConcurrentQueue &other)
	{
		if (this != &other)
		{
			std::scoped_lock lock(mutex_, other.mutex_);
			std::swap(queue_, other.queue_);

			if (!queue_.empty())
			{
				cond_.notify_all();
			}
			if (!other.queue_.empty())
			{
				other.cond_.notify_all();
			}
		}
	}

	friend void swap(ConcurrentQueue &a, ConcurrentQueue &b)
	{
		a.swap(b);
	}

	void clear()
	{
		std::lock_guard<std::mutex> lock(mutex_);
		std::queue<T>().swap(queue_);
	}

	bool empty() const
	{
		std::lock_guard<std::mutex> lock(mutex_);
		return queue_.empty();
	}

	size_t size() const
	{
		std::lock_guard<std::mutex> lock(mutex_);
		return queue_.size();
	}

	std::optional<T> try_peek() const
	{
		std::lock_guard<std::mutex> lock(mutex_);
		if (queue_.empty())
		{
			return std::nullopt;
		}
		return queue_.front();
	}
};


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
		return (uint32_t)(size_t)pthread_self();
#endif
    }

public:
    Thread() :
        handle{}
    {

    }

    template <class T>
    Thread(T task)
    {
		Start(task);
    }

    ~Thread()
    {
		Join();
    }

    template <class T>
    void Start(T task)
    {
        handle = std::jthread{ task };
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
		std::swap(handle, other.handle);
    }

protected:
    std::jthread handle;
};

using Task = std::function<void()>;

class TaskThread
{
public:
	TaskThread() :
	    size{},
	    thread{},
	    exited{}
    {
		thread = std::move(std::thread{[=, this]() {
            while (true)
            {
				size.wait(0);
				if (exited)
				{
					break;
				}
     //           if (size == 0xffffffff)
     //           {
					//break;
     //           }
				Task task{};
                if (tasks.try_dequeue(task))
                {
					task();
                }
                else
                {
					size = 0;
                }
            }
		}});
    }

    ~TaskThread()
    {
		exited = true;
		size = 0xffffffff;
		size.notify_one();
		Join();
    }

	template <class T>
	auto Enqueue(T task) -> std::future<decltype(task())>
	{
		auto wrapper = std::make_shared<std::packaged_task<decltype(task())()>>(std::move(task));
		{
			Task t = [=]() -> void { (*wrapper)(); };
            if (tasks.enqueue(std::move(t)))
            {
				size++;
				size.notify_one();
            }
		}
		return wrapper->get_future();
	}

    const std::atomic<uint32_t> &TaskSize() const
    {
		return size;
    }

    void RemoveTasks()
    {
		size = 0;
		ConcurrentQueue<Task> empty;
		tasks.swap(empty);
    }

    void Join()
    {
        if (thread.joinable())
        {
			thread.join();
        }
    }

protected:
	std::atomic_uint32_t size;

	ConcurrentQueue<Task> tasks;

	std::thread thread;

    bool exited;
};

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

    void SetDebugDescription(uint32_t index, const std::string &description)
	{
		threads[index].SetDebugDescription(description);
	}

    void OnNotify(const std::function<void()> &value)
    {
		notify = value;
    }

protected:
    std::vector<Thread> threads;

    std::atomic<uint32_t> taskRef;

    std::atomic<bool> tasked;

    std::function<void()> notify;

    std::condition_variable condition;

    std::queue<Task> tasks;

    std::mutex mutex;

    bool stopping{ false };
};

class IMMORTAL_API Async
{
public:
	static void Init(const uint32_t threadCount = std::thread::hardware_concurrency())
    {
		threadPool.reset(new ThreadPool{threadCount});
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
