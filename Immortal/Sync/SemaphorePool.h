#pragma once

#include "Semaphore.h"
#include "Render/Render.h"

namespace Immortal
{

class SemaphorePool
{
public:
    SemaphorePool() { }

    ~SemaphorePool() { }
};

using SuperSemaphorePool = SemaphorePool;

}
