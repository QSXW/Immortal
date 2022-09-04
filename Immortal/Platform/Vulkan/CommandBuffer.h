#pragma once

#include "Common.h"
#include "Buffer.h"
#include "Shader.h"
#include "Interface/IObject.h"

namespace Immortal
{
namespace Vulkan
{

class CommandPool;
class CommandBuffer : public IObject
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

    using Primitive = VkCommandBuffer;
    VKCPP_OPERATOR_HANDLE()

public:
    CommandBuffer(CommandPool *cmdPool, Level level);

    ~CommandBuffer();

    VkResult Begin(Usage flags = Usage::OneTimeSubmit, CommandBuffer *primaryCommandBuffer = nullptr);

    VkResult End();

    VkResult Execute();

    VkResult Reset(ResetMode reset_mode);

    bool Recording() const
    {
        return state == State::Recording;
    }

    bool Executable() const
    {
        return state == State::Executable;
    }

    bool ReadyToSubmit() const
    {
        return count == 10000;
    }

    bool IsRecorded() const
    {
        return count > 0;
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
        __Record();
        vkCmdPipelineBarrier(
            handle,
            srcStageMask, dstStageMask,
            dependencyFlags,
            memoryBarrierCount, pMemoryBarriers,
            bufferMemoryBarrierCount, pBufferMemoryBarriers,
            imageMemoryBarrierCount, pImageMemoryBarriers
        );
    }

    void PipelineImageBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, const VkImageMemoryBarrier *pImageMemoryBarriers, uint32_t imageMemoryBarrerCount = 1)
    {
        __Record();
        PipelineBarrier(srcStageMask, dstStageMask, 0, 0, nullptr, 0, nullptr, imageMemoryBarrerCount, pImageMemoryBarriers);
    }

    void BlitImage(VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageBlit *pRegions, VkFilter filter)
    {
        __Record();
        vkCmdBlitImage(handle, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions, filter);
    }

    void CopyImage(VkImage srcImage,
                   VkImageLayout srcImageLayout,
                   VkImage dstImage,
                   VkImageLayout dstImageLayout,
                   uint32_t regionCount,
                   const VkImageCopy *pRegions)
    {
        __Record();
        vkCmdCopyImage(
            handle,
            srcImage,
            srcImageLayout,
            dstImage,
            dstImageLayout,
            regionCount,
            pRegions
        );
    }

    void CopyBufferToImage(VkBuffer                 srcBuffer,
                           VkImage                  dstImage,
                           VkImageLayout            dstImageLayout,
                           uint32_t                 regionCount,
                           const VkBufferImageCopy *pRegions)
    {
        __Record();
        vkCmdCopyBufferToImage(handle, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions);
    }

    void BeginRenderPass(const VkRenderPassBeginInfo *pBeginInfo, VkSubpassContents contents)
    {
        __Record();
        vkCmdBeginRenderPass(handle, pBeginInfo, contents);
    }

    void EndRenderPass()
    {
        __Record();
        vkCmdEndRenderPass(handle);
    }

    void BindDescriptorSets(VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount, const VkDescriptorSet *pDescriptorSets, uint32_t dynamicOffsetCount, const uint32_t *pDynamicOffsets)
    {
        __Record();
        vkCmdBindDescriptorSets(handle, pipelineBindPoint, layout, firstSet, descriptorSetCount, pDescriptorSets, dynamicOffsetCount, pDynamicOffsets);
    }

    void BindPipeline(VkPipeline pipeline, VkPipelineBindPoint pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS)
    {
        __Record();
        vkCmdBindPipeline(handle, pipelineBindPoint, pipeline);
    }

    void BindVertexBuffers(uint32_t firstBinding, uint32_t bindingCount, const VkBuffer* pBuffers, const VkDeviceSize* pOffsets)
    {
        __Record();
        vkCmdBindVertexBuffers(handle, firstBinding, bindingCount, pBuffers, pOffsets);
    }

    void BindVertexBuffers(Buffer *buffer)
    {
        VkBuffer buffers[1]     = { *buffer };
        VkDeviceSize offsets[1] = { buffer->Offset() };
        BindVertexBuffers(0, 1, buffers, offsets);
    }

    void BindIndexBuffer(VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType = VK_INDEX_TYPE_UINT32)
    {   
        __Record();
        vkCmdBindIndexBuffer(handle, buffer, offset, indexType);
    }

    void BindIndexBuffer(Buffer *buffer, VkIndexType indexType = VK_INDEX_TYPE_UINT32)
    {
        BindIndexBuffer(*buffer, 0, indexType);
    }

    template <Buffer::Type type>
    constexpr void BindBuffer(std::shared_ptr<Buffer> &buffer)
    {
        if constexpr (type == Buffer::Type::Vertex)
        {
            BindVertexBuffers(buffer.get());
        }
        if constexpr (type == Buffer::Type::Index)
        {
            BindIndexBuffer(buffer.get());
        }
    }

    void PushConstants(VkPipelineLayout pipelineLayout, Shader::Stage stage, uint32_t offset, uint32_t size, const void *data)
    {
        __Record();
        vkCmdPushConstants(handle, pipelineLayout, (VkShaderStageFlags)stage, offset, size, data);
    }

    void SetViewport(float width, float height, float minDepth = 0.0f, float maxDepth = 1.0f)
    {
        VkViewport viewport{};
        viewport.width    = width;
        viewport.height   = height;
        viewport.minDepth = minDepth;
        viewport.maxDepth = maxDepth;

        __Record();
        vkCmdSetViewport(handle, 0, 1, &viewport);
    }

    void SetScissor(uint32_t width, uint32_t height, VkOffset2D offset = { 0, 0 })
    {
        VkRect2D scissor{};
        scissor.extent.width  = width;
        scissor.extent.height = height;
        scissor.offset        = offset;

        __Record();
        vkCmdSetScissor(handle, 0, 1, &scissor);
    }

    void Draw(uint32_t indexCount)
    {    
        __Record();
        vkCmdDrawIndexed(handle, indexCount, 1, 0, 0, 0);
    }

    void Dispatch(uint32_t nGroupX, uint32_t nGroupY, uint32_t nGroupZ)
    {
        __Record();
        vkCmdDispatch(handle, nGroupX, nGroupY, nGroupZ);
    }

    void DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance)
    {
        __Record();
        vkCmdDrawIndexed(handle, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    }

    void BeginVideoCoding(const VkVideoBeginCodingInfoKHR *pBeginInfo)
    {
        __Record();
        vkCmdBeginVideoCodingKHR(handle, pBeginInfo);
    }

    void EndVideoCoding(const VkVideoEndCodingInfoKHR *pEndInfo)
    {
        __Record();
        vkCmdEndVideoCodingKHR(handle, pEndInfo);
    }

    void Decode(const VkVideoDecodeInfoKHR *pFrameInfo)
    {
        __Record();
        vkCmdDecodeVideoKHR(handle, pFrameInfo);
    }

    void Begin(const VkVideoBeginCodingInfoKHR *pBeginInfo)
    {
        __Record();
        BeginVideoCoding(pBeginInfo);
    }

    void End(const VkVideoEndCodingInfoKHR *pEndInfo)
    {
        __Record();
        EndVideoCoding(pEndInfo);
    }

    void Reset(VkCommandBufferResetFlags flags)
    {
        vkResetCommandBuffer(handle, flags);
    }

protected:
    void __Record() 
    {
        count++;
    }

private:
    CommandPool *commandPool{ nullptr };

    std::atomic<uint32_t> count;

    State state{ State::Initial };

    Level level{ Level::Primary };
};

}
}
