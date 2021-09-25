#include "impch.h"
#include "ThreadManager.h"

namespace Immortal
{

std::unique_ptr<ThreadPool> ThreadManager::threadPool{ nullptr };

void ThreadManager::INIT()
{
    int count = std::thread::hardware_concurrency();

    LOG::INFO("Creating {0} Thread(s)", count);

    threadPool.reset(new ThreadPool{ count });
}

}
