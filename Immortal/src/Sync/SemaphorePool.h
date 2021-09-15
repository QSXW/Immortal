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

    virtual Semaphore Request() { return nullptr; }

    virtual void Reset() { }

    template <class ... Args>
    static inline Ref<SemaphorePool> Create(Args&& ... args);
};

}
