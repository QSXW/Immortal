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

    virtual void Reset() { }

    template <class ... Args>
    static inline std::shared_ptr<SemaphorePool> Create(Args&& ... args);
};

using SuperSemaphorePool = SemaphorePool;

}
