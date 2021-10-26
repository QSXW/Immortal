#pragma once

#include "Common.h"

namespace Immortal
{
namespace Vulkan
{

class CommandPool;
class CommandBuffer
{
public:
    enum class ResetMode
    {
        ResetPool,
        ResetIndividually,
        AlwaysAllocated
    };

    enum class State
    {
        Invalid,
        Initial,
        Recording,
        Executable
    };

    enum class Usage : int
    {
        
    };

    VkResult reset(ResetMode reset_mode);

public:
    CommandBuffer(CommandPool *cmdPool, Level level);

    ~CommandBuffer();

    VkResult Begin(VkCommandBufferUsageFlags flags =  VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, CommandBuffer *primaryCommandBuffer = nullptr);

    VkResult End();

    VkCommandBuffer &Handle()
    {
        return handle;
    }

    bool Recoding()
    {
        return state == State::Recording;
    }

    void PipelineBarrier(VkPipelineStageFlags         srcStageMask,
                         VkPipelineStageFlags         dstStageMask,
                         VkDependencyFlags            dependencyFlags,
                         uint32_t                     memoryBarrierCount,
                         const VkMemoryBarrier       *pMemoryBarriers,
                         uint32_t                     bufferMemoryBarrierCount,
                         const VkBufferMemoryBarrier *pBufferMemoryBarriers,
                         uint32_t                     imageMemoryBarrierCount,
                         const VkImageMemoryBarrier  *pImageMemoryBarriers)
    {
        vkCmdPipelineBarrier(
            handle,
            srcStageMask, dstStageMask,
            dependencyFlags,
            memoryBarrierCount, pMemoryBarriers,
            bufferMemoryBarrierCount, pBufferMemoryBarriers,
            imageMemoryBarrierCount, pImageMemoryBarriers
        );
    }

    void CopyBufferToImage(VkBuffer                 srcBuffer,
                           VkImage                  dstImage,
                           VkImageLayout            dstImageLayout,
                           uint32_t                 regionCount,
                           const VkBufferImageCopy *pRegions)
    {
        vkCmdCopyBufferToImage(handle, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions);
    }

private:
    CommandPool *commandPool{ nullptr };

    State state{ State::Initial };

    VkCommandBuffer handle{ VK_NULL_HANDLE };

    Level level{ Level::Primary };
};

}
}
