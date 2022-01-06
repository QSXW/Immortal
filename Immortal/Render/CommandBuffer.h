#pragma once

#include "Core.h"

namespace Immortal
{
    /**
     * @breif Command Buffers - Deferring the work
     * 
     * OpenGL is immediate (ignoring display lists)
     *  - Driver does not know how much work is incoming
     *  - Has to guess
     *  - Bad!
     * 
     * Vulkan and DirectX splits recording of work from submission of work
     *  - Removes guesswork from driver
     *  - Reducing hitching
     *  - Helps eliminate unexplained resouce usage
     *
     * Always belong to a Command Pool
     *  - Buffers are allocated from pool
     *  - Pools Provide lightweight synchronization
     *  - Pools can be reset, reclaming all resources
     *  - Two flavours of pool
     *    - individual reset of command buffers 
     *    - Gourp reset only
     * 
     * Are how you send things to the CPU and to be calculated on the GPU
     *  @Reference <a href="https://www.youtube.com/watch?v=X8Ob_b3ANzs">Ref</a>
     */
    class IMMORTAL_API CommandBuffer
    {

    };

}
