#include "Async.h"

namespace Immortal
{

std::unique_ptr<ThreadPool> Async::threadPool{ nullptr };

ThreadPool::ThreadPool(uint32_t numThreads) :
    taskRef{0}
{
    threads.reserve(numThreads);
    for (int i = 0; i < numThreads; i++)
    {
        threads.emplace_back([=, this]() -> void {
            while (true)
            {
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

                    task = std::move(tasks.front());
					tasks.pop();
                }
                task();
                taskRef--;
				if (taskRef.load() == 0)
				{
					tasked.store(true);
					tasked.notify_all();
				}
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

void ThreadPool::Join()
{
    tasked.wait(false);
}

void ThreadPool::RemoveTasks()
{
    {
		std::unique_lock<std::mutex> lock{mutex};
		while (!tasks.empty())
		{
			tasks.pop();
			taskRef--;
		}
    }
    if (taskRef.load() == 0)
    {
		tasked.store(true);
		tasked.notify_all();
    }
}

const std::atomic<uint32_t> &ThreadPool::TaskSize() const
{
	return taskRef;
}

}
