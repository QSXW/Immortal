#pragma once

#include "Common.h"
#include "Buffer.h"
#include "Pipeline.h"

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
        Executable,
        Pending
    };

    enum class Usage
    {
        OneTimeSubmit      = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        RenderPassContinue = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT,
        SimultaneousUse    = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT
    };

    VkResult reset(ResetMode reset_mode);

public:
    CommandBuffer(CommandPool *cmdPool, Level level);

    ~CommandBuffer();

    VkResult Begin(Usage flags = Usage::OneTimeSubmit, CommandBuffer *primaryCommandBuffer = nullptr);

    VkResult End();

    VkResult Execute();

    VkCommandBuffer &Handle()
    {
        return handle;
    }

    operator VkCommandBuffer&()
    {
        return handle;
    }

    operator VkCommandBuffer() const
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

    void BeginRenderPass(const VkRenderPassBeginInfo *pBeginInfo, VkSubpassContents contents)
    {
        vkCmdBeginRenderPass(handle, pBeginInfo, contents);
    }

    void EndRenderPass()
    {
        vkCmdEndRenderPass(handle);
    }

    void BindVertexBuffers(std::shared_ptr<Buffer> &buffer)
    {
        vkCmdBindVertexBuffers(handle, 0, 1, &buffer->Handle(), &buffer->Offset());
    }

    void BindIndexBuffer(std::shared_ptr<Buffer> &buffer)
    {
        vkCmdBindIndexBuffer(handle, *buffer, buffer->Offset(), VK_INDEX_TYPE_UINT32);
    }

    void Draw(uint32_t indexCount)
    {
        vkCmdDrawIndexed(handle, indexCount, 1, 0, 0, 0);
    }

    void DrawIndexed(std::shared_ptr<Buffer> &buffer)
    {
        BindIndexBuffer(buffer);
        Draw(buffer->Size());
    }

    void Draw(std::shared_ptr<Pipeline> &pipeline)
    {
        BindVertexBuffers(pipeline->Get<Buffer::Type::Vertex>());
        DrawIndexed(pipeline->Get<Buffer::Type::Index>());
    }

private:
    CommandPool *commandPool{ nullptr };

    State state{ State::Initial };

    VkCommandBuffer handle{ VK_NULL_HANDLE };

    Level level{ Level::Primary };
};

}
}
