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

    void BindVertexBuffer(std::shared_ptr<Buffer> &buffer)
    {
        vkCmdBindVertexBuffers(handle, 0, 1, &buffer->Handle(), &buffer->Offset());
    }

    void BindIndexBuffer(std::shared_ptr<Buffer> &buffer)
    {
        vkCmdBindIndexBuffer(handle, *buffer, buffer->Offset(), VK_INDEX_TYPE_UINT32);
    }

    template <Buffer::Type type>
    constexpr void BindBuffer(std::shared_ptr<Buffer> &buffer)
    {
        if constexpr (type == Buffer::Type::Vertex)
        {
            BindVertexBuffer(buffer);
            return;
        }
        if constexpr (type == Buffer::Type::Index)
        {
            BindIndexBuffer(buffer);
            return;
        }
    }

    void Bind(const std::shared_ptr<Pipeline> &pipeline)
    {
        const VkDescriptorSet descriptorSet = pipeline->GetDescriptorSet();
        vkCmdBindDescriptorSets(handle, pipeline->BindPoint(), pipeline->Layout(), 0, 1, &descriptorSet, 0, NULL);
        vkCmdBindPipeline(handle, pipeline->BindPoint(), *pipeline);
    }

    void PushConstants(const Pipeline &pipeline, Shader::Stage stage, uint32_t offset, uint32_t size, const void *data)
    {
        vkCmdPushConstants(handle, pipeline.Layout(), Shader::ConvertTo(stage), offset, size, data);
    }

    void SetViewport(float width, float height, float minDepth = 0.0f, float maxDepth = 1.0f)
    {
        VkViewport viewport{};
        viewport.width    = width;
        viewport.height   = height;
        viewport.minDepth = minDepth;
        viewport.maxDepth = maxDepth;

        vkCmdSetViewport(handle, 0, 1, &viewport);
    }

    void SetScissor(uint32_t width, uint32_t height, VkOffset2D offset = { 0, 0 })
    {
        VkRect2D scissor{};
        scissor.extent.width  = width;
        scissor.extent.height = height;
        scissor.offset        = offset;

        vkCmdSetScissor(handle, 0, 1, &scissor);
    }

    void Draw(uint32_t indexCount)
    {
        vkCmdDrawIndexed(handle, indexCount, 1, 0, 0, 0);
    }

    void DrawIndexed(std::shared_ptr<Buffer> &buffer)
    {
        BindIndexBuffer(buffer);
        Draw(buffer->Count());
    }

    void Draw(std::shared_ptr<Pipeline> &pipeline)
    {
        Bind(pipeline);
        BindVertexBuffer(pipeline->Get<Buffer::Type::Vertex>());
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
