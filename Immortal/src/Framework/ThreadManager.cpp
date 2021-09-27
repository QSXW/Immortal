#include "impch.h"
#include "ThreadManager.h"

namespace Immortal
{

std::unique_ptr<ThreadPool> Async::threadPool{ nullptr };

void Async::INIT()
{
    int count = std::thread::hardware_concurrency();

    LOG::INFO("Creating {0} Thread(s)", count);

    threadPool.reset(new ThreadPool{ count });
}

}
