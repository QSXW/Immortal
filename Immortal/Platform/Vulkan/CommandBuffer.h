#pragma once

#include "Common.h"
#include "Handle.h"
#include "Buffer.h"
#include "Shader.h"
#include "Interface/IObject.h"

namespace Immortal
{
namespace Vulkan
{

class CommandPool;
class CommandBuffer : public IObject, public Handle<VkCommandBuffer>
{
public:
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

    VKCPP_SWAPPABLE(CommandBuffer)

public:
    VkResult BeginCommandBuffer(VkCommandBufferBeginInfo const *pBeginInfo)
    {
        __Record();
        return vkBeginCommandBuffer(handle, pBeginInfo);
    }

    VkResult EndCommandBuffer()
    {
        __Record();
        return vkEndCommandBuffer(handle);
    }

    VkResult ResetCommandBuffer(VkCommandBufferResetFlags flags)
    {
        __Record();
        return vkResetCommandBuffer(handle, flags);
    }

    void BindPipeline(VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline)
    {
        __Record();
        vkCmdBindPipeline(handle, pipelineBindPoint, pipeline);
    }

    void SetViewport(uint32_t firstViewport, uint32_t viewportCount, VkViewport const *pViewports)
    {
        __Record();
        vkCmdSetViewport(handle, firstViewport, viewportCount, pViewports);
    }

    void SetScissor(uint32_t firstScissor, uint32_t scissorCount, VkRect2D const *pScissors)
    {
        __Record();
        vkCmdSetScissor(handle, firstScissor, scissorCount, pScissors);
    }

    void SetLineWidth(float lineWidth)
    {
        __Record();
        vkCmdSetLineWidth(handle, lineWidth);
    }

    void SetDepthBias(float depthBiasConstantFactor, float depthBiasClamp, float depthBiasSlopeFactor)
    {
        __Record();
        vkCmdSetDepthBias(handle, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
    }

    void SetBlendConstants(float const *blendConstants)
    {
        __Record();
        vkCmdSetBlendConstants(handle, blendConstants);
    }

    void SetDepthBounds(float minDepthBounds, float maxDepthBounds)
    {
        __Record();
        vkCmdSetDepthBounds(handle, minDepthBounds, maxDepthBounds);
    }

    void SetStencilCompareMask(VkStencilFaceFlags faceMask, uint32_t compareMask)
    {
        __Record();
        vkCmdSetStencilCompareMask(handle, faceMask, compareMask);
    }

    void SetStencilWriteMask(VkStencilFaceFlags faceMask, uint32_t writeMask)
    {
        __Record();
        vkCmdSetStencilWriteMask(handle, faceMask, writeMask);
    }

    void SetStencilReference(VkStencilFaceFlags faceMask, uint32_t reference)
    {
        __Record();
        vkCmdSetStencilReference(handle, faceMask, reference);
    }

    void BindDescriptorSets(VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount, VkDescriptorSet const *pDescriptorSets, uint32_t dynamicOffsetCount, uint32_t const *pDynamicOffsets)
    {
        __Record();
        vkCmdBindDescriptorSets(handle, pipelineBindPoint, layout, firstSet, descriptorSetCount, pDescriptorSets, dynamicOffsetCount, pDynamicOffsets);
    }

    void BindIndexBuffer(VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType)
    {
        __Record();
        vkCmdBindIndexBuffer(handle, buffer, offset, indexType);
    }

    void BindVertexBuffers(uint32_t firstBinding, uint32_t bindingCount, VkBuffer const *pBuffers, VkDeviceSize const *pOffsets)
    {
        __Record();
        vkCmdBindVertexBuffers(handle, firstBinding, bindingCount, pBuffers, pOffsets);
    }

    void Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
    {
        __Record();
        vkCmdDraw(handle, vertexCount, instanceCount, firstVertex, firstInstance);
    }

    void DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance)
    {
        __Record();
        vkCmdDrawIndexed(handle, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    }

    void DrawMultiEXT(uint32_t drawCount, VkMultiDrawInfoEXT const *pVertexInfo, uint32_t instanceCount, uint32_t firstInstance, uint32_t stride)
    {
        __Record();
        vkCmdDrawMultiEXT(handle, drawCount, pVertexInfo, instanceCount, firstInstance, stride);
    }

    void DrawMultiIndexedEXT(uint32_t drawCount, VkMultiDrawIndexedInfoEXT const *pIndexInfo, uint32_t instanceCount, uint32_t firstInstance, uint32_t stride, int32_t const *pVertexOffset)
    {
        __Record();
        vkCmdDrawMultiIndexedEXT(handle, drawCount, pIndexInfo, instanceCount, firstInstance, stride, pVertexOffset);
    }

    void DrawIndirect(VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride)
    {
        __Record();
        vkCmdDrawIndirect(handle, buffer, offset, drawCount, stride);
    }

    void DrawIndexedIndirect(VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride)
    {
        __Record();
        vkCmdDrawIndexedIndirect(handle, buffer, offset, drawCount, stride);
    }

    void Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
    {
        __Record();
        vkCmdDispatch(handle, groupCountX, groupCountY, groupCountZ);
    }

    void DispatchIndirect(VkBuffer buffer, VkDeviceSize offset)
    {
        __Record();
        vkCmdDispatchIndirect(handle, buffer, offset);
    }

    void SubpassShadingHUAWEI()
    {
        __Record();
        vkCmdSubpassShadingHUAWEI(handle);
    }

    void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount, VkBufferCopy const *pRegions)
    {
        __Record();
        vkCmdCopyBuffer(handle, srcBuffer, dstBuffer, regionCount, pRegions);
    }

    void CopyImage(VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, VkImageCopy const *pRegions)
    {
        __Record();
        vkCmdCopyImage(handle, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
    }

    void BlitImage(VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, VkImageBlit const *pRegions, VkFilter filter)
    {
        __Record();
        vkCmdBlitImage(handle, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions, filter);
    }

    void CopyBufferToImage(VkBuffer srcBuffer, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, VkBufferImageCopy const *pRegions)
    {
        __Record();
        vkCmdCopyBufferToImage(handle, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions);
    }

    void CopyImageToBuffer(VkImage srcImage, VkImageLayout srcImageLayout, VkBuffer dstBuffer, uint32_t regionCount, VkBufferImageCopy const *pRegions)
    {
        __Record();
        vkCmdCopyImageToBuffer(handle, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions);
    }

    void UpdateBuffer(VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize dataSize, void const *pData)
    {
        __Record();
        vkCmdUpdateBuffer(handle, dstBuffer, dstOffset, dataSize, pData);
    }

    void FillBuffer(VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize size, uint32_t data)
    {
        __Record();
        vkCmdFillBuffer(handle, dstBuffer, dstOffset, size, data);
    }

    void ClearColorImage(VkImage image, VkImageLayout imageLayout, VkClearColorValue const *pColor, uint32_t rangeCount, VkImageSubresourceRange const *pRanges)
    {
        __Record();
        vkCmdClearColorImage(handle, image, imageLayout, pColor, rangeCount, pRanges);
    }

    void ClearDepthStencilImage(VkImage image, VkImageLayout imageLayout, VkClearDepthStencilValue const *pDepthStencil, uint32_t rangeCount, VkImageSubresourceRange const *pRanges)
    {
        __Record();
        vkCmdClearDepthStencilImage(handle, image, imageLayout, pDepthStencil, rangeCount, pRanges);
    }

    void ClearAttachments(uint32_t attachmentCount, VkClearAttachment const *pAttachments, uint32_t rectCount, VkClearRect const *pRects)
    {
        __Record();
        vkCmdClearAttachments(handle, attachmentCount, pAttachments, rectCount, pRects);
    }

    void ResolveImage(VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, VkImageResolve const *pRegions)
    {
        __Record();
        vkCmdResolveImage(handle, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
    }

    void SetEvent(VkEvent event, VkPipelineStageFlags stageMask)
    {
        __Record();
        vkCmdSetEvent(handle, event, stageMask);
    }

    void ResetEvent(VkEvent event, VkPipelineStageFlags stageMask)
    {
        __Record();
        vkCmdResetEvent(handle, event, stageMask);
    }

    void WaitEvents(uint32_t eventCount, VkEvent const *pEvents, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, uint32_t memoryBarrierCount, VkMemoryBarrier const *pMemoryBarriers, uint32_t bufferMemoryBarrierCount, VkBufferMemoryBarrier const *pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, VkImageMemoryBarrier const *pImageMemoryBarriers)
    {
        __Record();
        vkCmdWaitEvents(handle, eventCount, pEvents, srcStageMask, dstStageMask, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
    }

    void PipelineBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags, uint32_t memoryBarrierCount, VkMemoryBarrier const *pMemoryBarriers, uint32_t bufferMemoryBarrierCount, VkBufferMemoryBarrier const *pBufferMemoryBarriers, uint32_t imageMemoryBarrierCount, VkImageMemoryBarrier const *pImageMemoryBarriers)
    {
        __Record();
        vkCmdPipelineBarrier(handle, srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount, pMemoryBarriers, bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
    }

    void BeginQuery(VkQueryPool queryPool, uint32_t query, VkQueryControlFlags flags)
    {
        __Record();
        vkCmdBeginQuery(handle, queryPool, query, flags);
    }

    void EndQuery(VkQueryPool queryPool, uint32_t query)
    {
        __Record();
        vkCmdEndQuery(handle, queryPool, query);
    }

    void BeginConditionalRenderingEXT(VkConditionalRenderingBeginInfoEXT const *pConditionalRenderingBegin)
    {
        __Record();
        vkCmdBeginConditionalRenderingEXT(handle, pConditionalRenderingBegin);
    }

    void EndConditionalRenderingEXT()
    {
        __Record();
        vkCmdEndConditionalRenderingEXT(handle);
    }

    void ResetQueryPool(VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount)
    {
        __Record();
        vkCmdResetQueryPool(handle, queryPool, firstQuery, queryCount);
    }

    void WriteTimestamp(VkPipelineStageFlagBits pipelineStage, VkQueryPool queryPool, uint32_t query)
    {
        __Record();
        vkCmdWriteTimestamp(handle, pipelineStage, queryPool, query);
    }

    void CopyQueryPoolResults(VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize stride, VkQueryResultFlags flags)
    {
        __Record();
        vkCmdCopyQueryPoolResults(handle, queryPool, firstQuery, queryCount, dstBuffer, dstOffset, stride, flags);
    }

    void PushConstants(VkPipelineLayout layout, VkShaderStageFlags stageFlags, uint32_t offset, uint32_t size, void const *pValues)
    {
        __Record();
        vkCmdPushConstants(handle, layout, stageFlags, offset, size, pValues);
    }

    void BeginRenderPass(VkRenderPassBeginInfo const *pRenderPassBegin, VkSubpassContents contents)
    {
        __Record();
        vkCmdBeginRenderPass(handle, pRenderPassBegin, contents);
    }

    void NextSubpass(VkSubpassContents contents)
    {
        __Record();
        vkCmdNextSubpass(handle, contents);
    }

    void EndRenderPass()
    {
        __Record();
        vkCmdEndRenderPass(handle);
    }

    void ExecuteCommands(uint32_t commandBufferCount, VkCommandBuffer const *pCommandBuffers)
    {
        __Record();
        vkCmdExecuteCommands(handle, commandBufferCount, pCommandBuffers);
    }

    void DebugMarkerBeginEXT(VkDebugMarkerMarkerInfoEXT const *pMarkerInfo)
    {
        __Record();
        vkCmdDebugMarkerBeginEXT(handle, pMarkerInfo);
    }

    void DebugMarkerEndEXT()
    {
        __Record();
        vkCmdDebugMarkerEndEXT(handle);
    }

    void DebugMarkerInsertEXT(VkDebugMarkerMarkerInfoEXT const *pMarkerInfo)
    {
        __Record();
        vkCmdDebugMarkerInsertEXT(handle, pMarkerInfo);
    }

    void ExecuteGeneratedCommandsNV(VkBool32 isPreprocessed, VkGeneratedCommandsInfoNV const *pGeneratedCommandsInfo)
    {
        __Record();
        vkCmdExecuteGeneratedCommandsNV(handle, isPreprocessed, pGeneratedCommandsInfo);
    }

    void PreprocessGeneratedCommandsNV(VkGeneratedCommandsInfoNV const *pGeneratedCommandsInfo)
    {
        __Record();
        vkCmdPreprocessGeneratedCommandsNV(handle, pGeneratedCommandsInfo);
    }

    void BindPipelineShaderGroupNV(VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline, uint32_t groupIndex)
    {
        __Record();
        vkCmdBindPipelineShaderGroupNV(handle, pipelineBindPoint, pipeline, groupIndex);
    }

    void PushDescriptorSetKHR(VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout layout, uint32_t set, uint32_t descriptorWriteCount, VkWriteDescriptorSet const *pDescriptorWrites)
    {
        __Record();
        vkCmdPushDescriptorSetKHR(handle, pipelineBindPoint, layout, set, descriptorWriteCount, pDescriptorWrites);
    }

    void SetDeviceMask(uint32_t deviceMask)
    {
        __Record();
        vkCmdSetDeviceMask(handle, deviceMask);
    }

    void DispatchBase(uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
    {
        __Record();
        vkCmdDispatchBase(handle, baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
    }

    void PushDescriptorSetWithTemplateKHR(VkDescriptorUpdateTemplate descriptorUpdateTemplate, VkPipelineLayout layout, uint32_t set, void const *pData)
    {
        __Record();
        vkCmdPushDescriptorSetWithTemplateKHR(handle, descriptorUpdateTemplate, layout, set, pData);
    }

    void SetViewportWScalingNV(uint32_t firstViewport, uint32_t viewportCount, VkViewportWScalingNV const *pViewportWScalings)
    {
        __Record();
        vkCmdSetViewportWScalingNV(handle, firstViewport, viewportCount, pViewportWScalings);
    }

    void SetDiscardRectangleEXT(uint32_t firstDiscardRectangle, uint32_t discardRectangleCount, VkRect2D const *pDiscardRectangles)
    {
        __Record();
        vkCmdSetDiscardRectangleEXT(handle, firstDiscardRectangle, discardRectangleCount, pDiscardRectangles);
    }

    void SetSampleLocationsEXT(VkSampleLocationsInfoEXT const *pSampleLocationsInfo)
    {
        __Record();
        vkCmdSetSampleLocationsEXT(handle, pSampleLocationsInfo);
    }

    void BeginDebugUtilsLabelEXT(VkDebugUtilsLabelEXT const *pLabelInfo)
    {
        __Record();
        vkCmdBeginDebugUtilsLabelEXT(handle, pLabelInfo);
    }

    void EndDebugUtilsLabelEXT()
    {
        __Record();
        vkCmdEndDebugUtilsLabelEXT(handle);
    }

    void InsertDebugUtilsLabelEXT(VkDebugUtilsLabelEXT const *pLabelInfo)
    {
        __Record();
        vkCmdInsertDebugUtilsLabelEXT(handle, pLabelInfo);
    }

    void WriteBufferMarkerAMD(VkPipelineStageFlagBits pipelineStage, VkBuffer dstBuffer, VkDeviceSize dstOffset, uint32_t marker)
    {
        __Record();
        vkCmdWriteBufferMarkerAMD(handle, pipelineStage, dstBuffer, dstOffset, marker);
    }

    void BeginRenderPass2(VkRenderPassBeginInfo const *pRenderPassBegin, VkSubpassBeginInfo const *pSubpassBeginInfo)
    {
        __Record();
        vkCmdBeginRenderPass2(handle, pRenderPassBegin, pSubpassBeginInfo);
    }

    void NextSubpass2(VkSubpassBeginInfo const *pSubpassBeginInfo, VkSubpassEndInfo const *pSubpassEndInfo)
    {
        __Record();
        vkCmdNextSubpass2(handle, pSubpassBeginInfo, pSubpassEndInfo);
    }

    void EndRenderPass2(VkSubpassEndInfo const *pSubpassEndInfo)
    {
        __Record();
        vkCmdEndRenderPass2(handle, pSubpassEndInfo);
    }

    void DrawIndirectCount(VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride)
    {
        __Record();
        vkCmdDrawIndirectCount(handle, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    }

    void DrawIndexedIndirectCount(VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride)
    {
        __Record();
        vkCmdDrawIndexedIndirectCount(handle, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    }

    void SetCheckpointNV(void const *pCheckpointMarker)
    {
        __Record();
        vkCmdSetCheckpointNV(handle, pCheckpointMarker);
    }

    void BindTransformFeedbackBuffersEXT(uint32_t firstBinding, uint32_t bindingCount, VkBuffer const *pBuffers, VkDeviceSize const *pOffsets, VkDeviceSize const *pSizes)
    {
        __Record();
        vkCmdBindTransformFeedbackBuffersEXT(handle, firstBinding, bindingCount, pBuffers, pOffsets, pSizes);
    }

    void BeginTransformFeedbackEXT(uint32_t firstCounterBuffer, uint32_t counterBufferCount, VkBuffer const *pCounterBuffers, VkDeviceSize const *pCounterBufferOffsets)
    {
        __Record();
        vkCmdBeginTransformFeedbackEXT(handle, firstCounterBuffer, counterBufferCount, pCounterBuffers, pCounterBufferOffsets);
    }

    void EndTransformFeedbackEXT(uint32_t firstCounterBuffer, uint32_t counterBufferCount, VkBuffer const *pCounterBuffers, VkDeviceSize const *pCounterBufferOffsets)
    {
        __Record();
        vkCmdEndTransformFeedbackEXT(handle, firstCounterBuffer, counterBufferCount, pCounterBuffers, pCounterBufferOffsets);
    }

    void BeginQueryIndexedEXT(VkQueryPool queryPool, uint32_t query, VkQueryControlFlags flags, uint32_t index)
    {
        __Record();
        vkCmdBeginQueryIndexedEXT(handle, queryPool, query, flags, index);
    }

    void EndQueryIndexedEXT(VkQueryPool queryPool, uint32_t query, uint32_t index)
    {
        __Record();
        vkCmdEndQueryIndexedEXT(handle, queryPool, query, index);
    }

    void DrawIndirectByteCountEXT(uint32_t instanceCount, uint32_t firstInstance, VkBuffer counterBuffer, VkDeviceSize counterBufferOffset, uint32_t counterOffset, uint32_t vertexStride)
    {
        __Record();
        vkCmdDrawIndirectByteCountEXT(handle, instanceCount, firstInstance, counterBuffer, counterBufferOffset, counterOffset, vertexStride);
    }

    void SetExclusiveScissorNV(uint32_t firstExclusiveScissor, uint32_t exclusiveScissorCount, VkRect2D const *pExclusiveScissors)
    {
        __Record();
        vkCmdSetExclusiveScissorNV(handle, firstExclusiveScissor, exclusiveScissorCount, pExclusiveScissors);
    }

    void BindShadingRateImageNV(VkImageView imageView, VkImageLayout imageLayout)
    {
        __Record();
        vkCmdBindShadingRateImageNV(handle, imageView, imageLayout);
    }

    void SetViewportShadingRatePaletteNV(uint32_t firstViewport, uint32_t viewportCount, VkShadingRatePaletteNV const *pShadingRatePalettes)
    {
        __Record();
        vkCmdSetViewportShadingRatePaletteNV(handle, firstViewport, viewportCount, pShadingRatePalettes);
    }

    void SetCoarseSampleOrderNV(VkCoarseSampleOrderTypeNV sampleOrderType, uint32_t customSampleOrderCount, VkCoarseSampleOrderCustomNV const *pCustomSampleOrders)
    {
        __Record();
        vkCmdSetCoarseSampleOrderNV(handle, sampleOrderType, customSampleOrderCount, pCustomSampleOrders);
    }

    void DrawMeshTasksNV(uint32_t taskCount, uint32_t firstTask)
    {
        __Record();
        vkCmdDrawMeshTasksNV(handle, taskCount, firstTask);
    }

    void DrawMeshTasksIndirectNV(VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride)
    {
        __Record();
        vkCmdDrawMeshTasksIndirectNV(handle, buffer, offset, drawCount, stride);
    }

    void DrawMeshTasksIndirectCountNV(VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride)
    {
        __Record();
        vkCmdDrawMeshTasksIndirectCountNV(handle, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    }

    void DrawMeshTasksEXT(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
    {
        __Record();
        vkCmdDrawMeshTasksEXT(handle, groupCountX, groupCountY, groupCountZ);
    }

    void DrawMeshTasksIndirectEXT(VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride)
    {
        __Record();
        vkCmdDrawMeshTasksIndirectEXT(handle, buffer, offset, drawCount, stride);
    }

    void DrawMeshTasksIndirectCountEXT(VkBuffer buffer, VkDeviceSize offset, VkBuffer countBuffer, VkDeviceSize countBufferOffset, uint32_t maxDrawCount, uint32_t stride)
    {
        __Record();
        vkCmdDrawMeshTasksIndirectCountEXT(handle, buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
    }

    void BindInvocationMaskHUAWEI(VkImageView imageView, VkImageLayout imageLayout)
    {
        __Record();
        vkCmdBindInvocationMaskHUAWEI(handle, imageView, imageLayout);
    }

    void CopyAccelerationStructureNV(VkAccelerationStructureNV dst, VkAccelerationStructureNV src, VkCopyAccelerationStructureModeKHR mode)
    {
        __Record();
        vkCmdCopyAccelerationStructureNV(handle, dst, src, mode);
    }

    void CopyAccelerationStructureKHR(VkCopyAccelerationStructureInfoKHR const *pInfo)
    {
        __Record();
        vkCmdCopyAccelerationStructureKHR(handle, pInfo);
    }

    void CopyAccelerationStructureToMemoryKHR(VkCopyAccelerationStructureToMemoryInfoKHR const *pInfo)
    {
        __Record();
        vkCmdCopyAccelerationStructureToMemoryKHR(handle, pInfo);
    }

    void CopyMemoryToAccelerationStructureKHR(VkCopyMemoryToAccelerationStructureInfoKHR const *pInfo)
    {
        __Record();
        vkCmdCopyMemoryToAccelerationStructureKHR(handle, pInfo);
    }

    void WriteAccelerationStructuresPropertiesKHR(uint32_t accelerationStructureCount, VkAccelerationStructureKHR const *pAccelerationStructures, VkQueryType queryType, VkQueryPool queryPool, uint32_t firstQuery)
    {
        __Record();
        vkCmdWriteAccelerationStructuresPropertiesKHR(handle, accelerationStructureCount, pAccelerationStructures, queryType, queryPool, firstQuery);
    }

    void WriteAccelerationStructuresPropertiesNV(uint32_t accelerationStructureCount, VkAccelerationStructureNV const *pAccelerationStructures, VkQueryType queryType, VkQueryPool queryPool, uint32_t firstQuery)
    {
        __Record();
        vkCmdWriteAccelerationStructuresPropertiesNV(handle, accelerationStructureCount, pAccelerationStructures, queryType, queryPool, firstQuery);
    }

    void BuildAccelerationStructureNV(VkAccelerationStructureInfoNV const *pInfo, VkBuffer instanceData, VkDeviceSize instanceOffset, VkBool32 update, VkAccelerationStructureNV dst, VkAccelerationStructureNV src, VkBuffer scratch, VkDeviceSize scratchOffset)
    {
        __Record();
        vkCmdBuildAccelerationStructureNV(handle, pInfo, instanceData, instanceOffset, update, dst, src, scratch, scratchOffset);
    }

    void TraceRaysKHR(VkStridedDeviceAddressRegionKHR const *pRaygenShaderBindingTable, VkStridedDeviceAddressRegionKHR const *pMissShaderBindingTable, VkStridedDeviceAddressRegionKHR const *pHitShaderBindingTable, VkStridedDeviceAddressRegionKHR const *pCallableShaderBindingTable, uint32_t width, uint32_t height, uint32_t depth)
    {
        __Record();
        vkCmdTraceRaysKHR(handle, pRaygenShaderBindingTable, pMissShaderBindingTable, pHitShaderBindingTable, pCallableShaderBindingTable, width, height, depth);
    }

    void TraceRaysNV(VkBuffer raygenShaderBindingTableBuffer, VkDeviceSize raygenShaderBindingOffset, VkBuffer missShaderBindingTableBuffer, VkDeviceSize missShaderBindingOffset, VkDeviceSize missShaderBindingStride, VkBuffer hitShaderBindingTableBuffer, VkDeviceSize hitShaderBindingOffset, VkDeviceSize hitShaderBindingStride, VkBuffer callableShaderBindingTableBuffer, VkDeviceSize callableShaderBindingOffset, VkDeviceSize callableShaderBindingStride, uint32_t width, uint32_t height, uint32_t depth)
    {
        __Record();
        vkCmdTraceRaysNV(handle, raygenShaderBindingTableBuffer, raygenShaderBindingOffset, missShaderBindingTableBuffer, missShaderBindingOffset, missShaderBindingStride, hitShaderBindingTableBuffer, hitShaderBindingOffset, hitShaderBindingStride, callableShaderBindingTableBuffer, callableShaderBindingOffset, callableShaderBindingStride, width, height, depth);
    }

    void TraceRaysIndirectKHR(VkStridedDeviceAddressRegionKHR const *pRaygenShaderBindingTable, VkStridedDeviceAddressRegionKHR const *pMissShaderBindingTable, VkStridedDeviceAddressRegionKHR const *pHitShaderBindingTable, VkStridedDeviceAddressRegionKHR const *pCallableShaderBindingTable, VkDeviceAddress indirectDeviceAddress)
    {
        __Record();
        vkCmdTraceRaysIndirectKHR(handle, pRaygenShaderBindingTable, pMissShaderBindingTable, pHitShaderBindingTable, pCallableShaderBindingTable, indirectDeviceAddress);
    }

    void TraceRaysIndirect2KHR(VkDeviceAddress indirectDeviceAddress)
    {
        __Record();
        vkCmdTraceRaysIndirect2KHR(handle, indirectDeviceAddress);
    }

    void SetRayTracingPipelineStackSizeKHR(uint32_t pipelineStackSize)
    {
        __Record();
        vkCmdSetRayTracingPipelineStackSizeKHR(handle, pipelineStackSize);
    }

    VkResult SetPerformanceMarkerINTEL(VkPerformanceMarkerInfoINTEL const *pMarkerInfo)
    {
        __Record();
        return vkCmdSetPerformanceMarkerINTEL(handle, pMarkerInfo);
    }

    VkResult SetPerformanceStreamMarkerINTEL(VkPerformanceStreamMarkerInfoINTEL const *pMarkerInfo)
    {
        __Record();
        return vkCmdSetPerformanceStreamMarkerINTEL(handle, pMarkerInfo);
    }

    VkResult SetPerformanceOverrideINTEL(VkPerformanceOverrideInfoINTEL const *pOverrideInfo)
    {
        __Record();
        return vkCmdSetPerformanceOverrideINTEL(handle, pOverrideInfo);
    }

    void SetLineStippleEXT(uint32_t lineStippleFactor, uint16_t lineStipplePattern)
    {
        __Record();
        vkCmdSetLineStippleEXT(handle, lineStippleFactor, lineStipplePattern);
    }

    void BuildAccelerationStructuresKHR(uint32_t infoCount, VkAccelerationStructureBuildGeometryInfoKHR const *pInfos, VkAccelerationStructureBuildRangeInfoKHR const * const*ppBuildRangeInfos)
    {
        __Record();
        vkCmdBuildAccelerationStructuresKHR(handle, infoCount, pInfos, ppBuildRangeInfos);
    }

    void BuildAccelerationStructuresIndirectKHR(uint32_t infoCount, VkAccelerationStructureBuildGeometryInfoKHR const *pInfos, VkDeviceAddress const *pIndirectDeviceAddresses, uint32_t const *pIndirectStrides, uint32_t const * const*ppMaxPrimitiveCounts)
    {
        __Record();
        vkCmdBuildAccelerationStructuresIndirectKHR(handle, infoCount, pInfos, pIndirectDeviceAddresses, pIndirectStrides, ppMaxPrimitiveCounts);
    }

    void SetCullMode(VkCullModeFlags cullMode)
    {
        __Record();
        vkCmdSetCullMode(handle, cullMode);
    }

    void SetFrontFace(VkFrontFace frontFace)
    {
        __Record();
        vkCmdSetFrontFace(handle, frontFace);
    }

    void SetPrimitiveTopology(VkPrimitiveTopology primitiveTopology)
    {
        __Record();
        vkCmdSetPrimitiveTopology(handle, primitiveTopology);
    }

    void SetViewportWithCount(uint32_t viewportCount, VkViewport const *pViewports)
    {
        __Record();
        vkCmdSetViewportWithCount(handle, viewportCount, pViewports);
    }

    void SetScissorWithCount(uint32_t scissorCount, VkRect2D const *pScissors)
    {
        __Record();
        vkCmdSetScissorWithCount(handle, scissorCount, pScissors);
    }

    void BindVertexBuffers2(uint32_t firstBinding, uint32_t bindingCount, VkBuffer const *pBuffers, VkDeviceSize const *pOffsets, VkDeviceSize const *pSizes, VkDeviceSize const *pStrides)
    {
        __Record();
        vkCmdBindVertexBuffers2(handle, firstBinding, bindingCount, pBuffers, pOffsets, pSizes, pStrides);
    }

    void SetDepthTestEnable(VkBool32 depthTestEnable)
    {
        __Record();
        vkCmdSetDepthTestEnable(handle, depthTestEnable);
    }

    void SetDepthWriteEnable(VkBool32 depthWriteEnable)
    {
        __Record();
        vkCmdSetDepthWriteEnable(handle, depthWriteEnable);
    }

    void SetDepthCompareOp(VkCompareOp depthCompareOp)
    {
        __Record();
        vkCmdSetDepthCompareOp(handle, depthCompareOp);
    }

    void SetDepthBoundsTestEnable(VkBool32 depthBoundsTestEnable)
    {
        __Record();
        vkCmdSetDepthBoundsTestEnable(handle, depthBoundsTestEnable);
    }

    void SetStencilTestEnable(VkBool32 stencilTestEnable)
    {
        __Record();
        vkCmdSetStencilTestEnable(handle, stencilTestEnable);
    }

    void SetStencilOp(VkStencilFaceFlags faceMask, VkStencilOp failOp, VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp)
    {
        __Record();
        vkCmdSetStencilOp(handle, faceMask, failOp, passOp, depthFailOp, compareOp);
    }

    void SetPatchControlPointsEXT(uint32_t patchControlPoints)
    {
        __Record();
        vkCmdSetPatchControlPointsEXT(handle, patchControlPoints);
    }

    void SetRasterizerDiscardEnable(VkBool32 rasterizerDiscardEnable)
    {
        __Record();
        vkCmdSetRasterizerDiscardEnable(handle, rasterizerDiscardEnable);
    }

    void SetDepthBiasEnable(VkBool32 depthBiasEnable)
    {
        __Record();
        vkCmdSetDepthBiasEnable(handle, depthBiasEnable);
    }

    void SetLogicOpEXT(VkLogicOp logicOp)
    {
        __Record();
        vkCmdSetLogicOpEXT(handle, logicOp);
    }

    void SetPrimitiveRestartEnable(VkBool32 primitiveRestartEnable)
    {
        __Record();
        vkCmdSetPrimitiveRestartEnable(handle, primitiveRestartEnable);
    }

    void SetTessellationDomainOriginEXT(VkTessellationDomainOrigin domainOrigin)
    {
        __Record();
        vkCmdSetTessellationDomainOriginEXT(handle, domainOrigin);
    }

    void SetDepthClampEnableEXT(VkBool32 depthClampEnable)
    {
        __Record();
        vkCmdSetDepthClampEnableEXT(handle, depthClampEnable);
    }

    void SetPolygonModeEXT(VkPolygonMode polygonMode)
    {
        __Record();
        vkCmdSetPolygonModeEXT(handle, polygonMode);
    }

    void SetRasterizationSamplesEXT(VkSampleCountFlagBits rasterizationSamples)
    {
        __Record();
        vkCmdSetRasterizationSamplesEXT(handle, rasterizationSamples);
    }

    void SetSampleMaskEXT(VkSampleCountFlagBits samples, VkSampleMask const *pSampleMask)
    {
        __Record();
        vkCmdSetSampleMaskEXT(handle, samples, pSampleMask);
    }

    void SetAlphaToCoverageEnableEXT(VkBool32 alphaToCoverageEnable)
    {
        __Record();
        vkCmdSetAlphaToCoverageEnableEXT(handle, alphaToCoverageEnable);
    }

    void SetAlphaToOneEnableEXT(VkBool32 alphaToOneEnable)
    {
        __Record();
        vkCmdSetAlphaToOneEnableEXT(handle, alphaToOneEnable);
    }

    void SetLogicOpEnableEXT(VkBool32 logicOpEnable)
    {
        __Record();
        vkCmdSetLogicOpEnableEXT(handle, logicOpEnable);
    }

    void SetColorBlendEnableEXT(uint32_t firstAttachment, uint32_t attachmentCount, VkBool32 const *pColorBlendEnables)
    {
        __Record();
        vkCmdSetColorBlendEnableEXT(handle, firstAttachment, attachmentCount, pColorBlendEnables);
    }

    void SetColorBlendEquationEXT(uint32_t firstAttachment, uint32_t attachmentCount, VkColorBlendEquationEXT const *pColorBlendEquations)
    {
        __Record();
        vkCmdSetColorBlendEquationEXT(handle, firstAttachment, attachmentCount, pColorBlendEquations);
    }

    void SetColorWriteMaskEXT(uint32_t firstAttachment, uint32_t attachmentCount, VkColorComponentFlags const *pColorWriteMasks)
    {
        __Record();
        vkCmdSetColorWriteMaskEXT(handle, firstAttachment, attachmentCount, pColorWriteMasks);
    }

    void SetRasterizationStreamEXT(uint32_t rasterizationStream)
    {
        __Record();
        vkCmdSetRasterizationStreamEXT(handle, rasterizationStream);
    }

    void SetConservativeRasterizationModeEXT(VkConservativeRasterizationModeEXT conservativeRasterizationMode)
    {
        __Record();
        vkCmdSetConservativeRasterizationModeEXT(handle, conservativeRasterizationMode);
    }

    void SetExtraPrimitiveOverestimationSizeEXT(float extraPrimitiveOverestimationSize)
    {
        __Record();
        vkCmdSetExtraPrimitiveOverestimationSizeEXT(handle, extraPrimitiveOverestimationSize);
    }

    void SetDepthClipEnableEXT(VkBool32 depthClipEnable)
    {
        __Record();
        vkCmdSetDepthClipEnableEXT(handle, depthClipEnable);
    }

    void SetSampleLocationsEnableEXT(VkBool32 sampleLocationsEnable)
    {
        __Record();
        vkCmdSetSampleLocationsEnableEXT(handle, sampleLocationsEnable);
    }

    void SetColorBlendAdvancedEXT(uint32_t firstAttachment, uint32_t attachmentCount, VkColorBlendAdvancedEXT const *pColorBlendAdvanced)
    {
        __Record();
        vkCmdSetColorBlendAdvancedEXT(handle, firstAttachment, attachmentCount, pColorBlendAdvanced);
    }

    void SetProvokingVertexModeEXT(VkProvokingVertexModeEXT provokingVertexMode)
    {
        __Record();
        vkCmdSetProvokingVertexModeEXT(handle, provokingVertexMode);
    }

    void SetLineRasterizationModeEXT(VkLineRasterizationModeEXT lineRasterizationMode)
    {
        __Record();
        vkCmdSetLineRasterizationModeEXT(handle, lineRasterizationMode);
    }

    void SetLineStippleEnableEXT(VkBool32 stippledLineEnable)
    {
        __Record();
        vkCmdSetLineStippleEnableEXT(handle, stippledLineEnable);
    }

    void SetDepthClipNegativeOneToOneEXT(VkBool32 negativeOneToOne)
    {
        __Record();
        vkCmdSetDepthClipNegativeOneToOneEXT(handle, negativeOneToOne);
    }

    void SetViewportWScalingEnableNV(VkBool32 viewportWScalingEnable)
    {
        __Record();
        vkCmdSetViewportWScalingEnableNV(handle, viewportWScalingEnable);
    }

    void SetViewportSwizzleNV(uint32_t firstViewport, uint32_t viewportCount, VkViewportSwizzleNV const *pViewportSwizzles)
    {
        __Record();
        vkCmdSetViewportSwizzleNV(handle, firstViewport, viewportCount, pViewportSwizzles);
    }

    void SetCoverageToColorEnableNV(VkBool32 coverageToColorEnable)
    {
        __Record();
        vkCmdSetCoverageToColorEnableNV(handle, coverageToColorEnable);
    }

    void SetCoverageToColorLocationNV(uint32_t coverageToColorLocation)
    {
        __Record();
        vkCmdSetCoverageToColorLocationNV(handle, coverageToColorLocation);
    }

    void SetCoverageModulationModeNV(VkCoverageModulationModeNV coverageModulationMode)
    {
        __Record();
        vkCmdSetCoverageModulationModeNV(handle, coverageModulationMode);
    }

    void SetCoverageModulationTableEnableNV(VkBool32 coverageModulationTableEnable)
    {
        __Record();
        vkCmdSetCoverageModulationTableEnableNV(handle, coverageModulationTableEnable);
    }

    void SetCoverageModulationTableNV(uint32_t coverageModulationTableCount, float const *pCoverageModulationTable)
    {
        __Record();
        vkCmdSetCoverageModulationTableNV(handle, coverageModulationTableCount, pCoverageModulationTable);
    }

    void SetShadingRateImageEnableNV(VkBool32 shadingRateImageEnable)
    {
        __Record();
        vkCmdSetShadingRateImageEnableNV(handle, shadingRateImageEnable);
    }

    void SetCoverageReductionModeNV(VkCoverageReductionModeNV coverageReductionMode)
    {
        __Record();
        vkCmdSetCoverageReductionModeNV(handle, coverageReductionMode);
    }

    void SetRepresentativeFragmentTestEnableNV(VkBool32 representativeFragmentTestEnable)
    {
        __Record();
        vkCmdSetRepresentativeFragmentTestEnableNV(handle, representativeFragmentTestEnable);
    }

    void CopyBuffer2(VkCopyBufferInfo2 const *pCopyBufferInfo)
    {
        __Record();
        vkCmdCopyBuffer2(handle, pCopyBufferInfo);
    }

    void CopyImage2(VkCopyImageInfo2 const *pCopyImageInfo)
    {
        __Record();
        vkCmdCopyImage2(handle, pCopyImageInfo);
    }

    void BlitImage2(VkBlitImageInfo2 const *pBlitImageInfo)
    {
        __Record();
        vkCmdBlitImage2(handle, pBlitImageInfo);
    }

    void CopyBufferToImage2(VkCopyBufferToImageInfo2 const *pCopyBufferToImageInfo)
    {
        __Record();
        vkCmdCopyBufferToImage2(handle, pCopyBufferToImageInfo);
    }

    void CopyImageToBuffer2(VkCopyImageToBufferInfo2 const *pCopyImageToBufferInfo)
    {
        __Record();
        vkCmdCopyImageToBuffer2(handle, pCopyImageToBufferInfo);
    }

    void ResolveImage2(VkResolveImageInfo2 const *pResolveImageInfo)
    {
        __Record();
        vkCmdResolveImage2(handle, pResolveImageInfo);
    }

    void SetFragmentShadingRateKHR(VkExtent2D const *pFragmentSize, VkFragmentShadingRateCombinerOpKHR const *combinerOps)
    {
        __Record();
        vkCmdSetFragmentShadingRateKHR(handle, pFragmentSize, combinerOps);
    }

    void SetFragmentShadingRateEnumNV(VkFragmentShadingRateNV shadingRate, VkFragmentShadingRateCombinerOpKHR const *combinerOps)
    {
        __Record();
        vkCmdSetFragmentShadingRateEnumNV(handle, shadingRate, combinerOps);
    }

    void SetVertexInputEXT(uint32_t vertexBindingDescriptionCount, VkVertexInputBindingDescription2EXT const *pVertexBindingDescriptions, uint32_t vertexAttributeDescriptionCount, VkVertexInputAttributeDescription2EXT const *pVertexAttributeDescriptions)
    {
        __Record();
        vkCmdSetVertexInputEXT(handle, vertexBindingDescriptionCount, pVertexBindingDescriptions, vertexAttributeDescriptionCount, pVertexAttributeDescriptions);
    }

    void SetColorWriteEnableEXT(uint32_t attachmentCount, VkBool32 const *pColorWriteEnables)
    {
        __Record();
        vkCmdSetColorWriteEnableEXT(handle, attachmentCount, pColorWriteEnables);
    }

    void SetEvent2(VkEvent event, VkDependencyInfo const *pDependencyInfo)
    {
        __Record();
        vkCmdSetEvent2(handle, event, pDependencyInfo);
    }

    void ResetEvent2(VkEvent event, VkPipelineStageFlags2 stageMask)
    {
        __Record();
        vkCmdResetEvent2(handle, event, stageMask);
    }

    void WaitEvents2(uint32_t eventCount, VkEvent const *pEvents, VkDependencyInfo const *pDependencyInfos)
    {
        __Record();
        vkCmdWaitEvents2(handle, eventCount, pEvents, pDependencyInfos);
    }

    void PipelineBarrier2(VkDependencyInfo const *pDependencyInfo)
    {
        __Record();
        vkCmdPipelineBarrier2(handle, pDependencyInfo);
    }

    void WriteTimestamp2(VkPipelineStageFlags2 stage, VkQueryPool queryPool, uint32_t query)
    {
        __Record();
        vkCmdWriteTimestamp2(handle, stage, queryPool, query);
    }

    void WriteBufferMarker2AMD(VkPipelineStageFlags2 stage, VkBuffer dstBuffer, VkDeviceSize dstOffset, uint32_t marker)
    {
        __Record();
        vkCmdWriteBufferMarker2AMD(handle, stage, dstBuffer, dstOffset, marker);
    }

    void DecodeVideoKHR(VkVideoDecodeInfoKHR const *pDecodeInfo)
    {
        __Record();
        vkCmdDecodeVideoKHR(handle, pDecodeInfo);
    }

    void BeginVideoCodingKHR(VkVideoBeginCodingInfoKHR const *pBeginInfo)
    {
        __Record();
        vkCmdBeginVideoCodingKHR(handle, pBeginInfo);
    }

    void ControlVideoCodingKHR(VkVideoCodingControlInfoKHR const *pCodingControlInfo)
    {
        __Record();
        vkCmdControlVideoCodingKHR(handle, pCodingControlInfo);
    }

    void EndVideoCodingKHR(VkVideoEndCodingInfoKHR const *pEndCodingInfo)
    {
        __Record();
        vkCmdEndVideoCodingKHR(handle, pEndCodingInfo);
    }

    void EncodeVideoKHR(VkVideoEncodeInfoKHR const *pEncodeInfo)
    {
        __Record();
        vkCmdEncodeVideoKHR(handle, pEncodeInfo);
    }

    void CuLaunchKernelNVX(VkCuLaunchInfoNVX const *pLaunchInfo)
    {
        __Record();
        vkCmdCuLaunchKernelNVX(handle, pLaunchInfo);
    }

    void BeginRendering(VkRenderingInfo const *pRenderingInfo)
    {
        __Record();
        vkCmdBeginRendering(handle, pRenderingInfo);
    }

    void EndRendering()
    {
        __Record();
        vkCmdEndRendering(handle);
    }

    void BuildMicromapsEXT(uint32_t infoCount, VkMicromapBuildInfoEXT const *pInfos)
    {
        __Record();
        vkCmdBuildMicromapsEXT(handle, infoCount, pInfos);
    }

    void CopyMicromapEXT(VkCopyMicromapInfoEXT const *pInfo)
    {
        __Record();
        vkCmdCopyMicromapEXT(handle, pInfo);
    }

    void CopyMicromapToMemoryEXT(VkCopyMicromapToMemoryInfoEXT const *pInfo)
    {
        __Record();
        vkCmdCopyMicromapToMemoryEXT(handle, pInfo);
    }

    void CopyMemoryToMicromapEXT(VkCopyMemoryToMicromapInfoEXT const *pInfo)
    {
        __Record();
        vkCmdCopyMemoryToMicromapEXT(handle, pInfo);
    }

    void WriteMicromapsPropertiesEXT(uint32_t micromapCount, VkMicromapEXT const *pMicromaps, VkQueryType queryType, VkQueryPool queryPool, uint32_t firstQuery)
    {
        __Record();
        vkCmdWriteMicromapsPropertiesEXT(handle, micromapCount, pMicromaps, queryType, queryPool, firstQuery);
    }

    void OpticalFlowExecuteNV(VkOpticalFlowSessionNV session, VkOpticalFlowExecuteInfoNV const *pExecuteInfo)
    {
        __Record();
        vkCmdOpticalFlowExecuteNV(handle, session, pExecuteInfo);
    }

public:
    CommandBuffer(CommandPool *commandPool = nullptr, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    ~CommandBuffer();

    void Destroy(CommandPool *commandPool);

    VkResult Begin(Usage flags = Usage::OneTimeSubmit, CommandBuffer *primaryCommandBuffer = nullptr, const VkCommandBufferInheritanceInfo *pInheritanceInfo = nullptr);

    VkResult End();

    VkResult Execute();

    VkResult Reset(VkCommandBufferResetFlags flags = VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

    void SetState(State _state)
    {
        state = _state;
    }

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

    void PipelineImageBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, const VkImageMemoryBarrier *pImageMemoryBarriers, uint32_t imageMemoryBarrerCount = 1)
    {
        PipelineBarrier(srcStageMask, dstStageMask, 0, 0, nullptr, 0, nullptr, imageMemoryBarrerCount, pImageMemoryBarriers);
    }

    void BindVertexBuffers(Buffer *buffer)
    {
        VkBuffer buffers[1]     = { *buffer };
        VkDeviceSize offsets[1] = { buffer->Offset() };
        BindVertexBuffers(0, 1, buffers, offsets);
    }

    void BindIndexBuffer(Buffer *buffer, VkIndexType indexType = VK_INDEX_TYPE_UINT32)
    {
        BindIndexBuffer(*buffer, buffer->Offset(), indexType);
    }

    void SetViewport(float width, float height, float minDepth = 0.0f, float maxDepth = 1.0f)
    {
        VkViewport viewport{};
        viewport.width    = width;
        viewport.height   = height;
        viewport.minDepth = minDepth;
        viewport.maxDepth = maxDepth;

        SetViewport(0, 1, &viewport);
    }

    void SetScissor(uint32_t width, uint32_t height, VkOffset2D offset = { 0, 0 })
    {
        VkRect2D scissor{};
        scissor.extent.width  = width;
        scissor.extent.height = height;
        scissor.offset        = offset;

        SetScissor(0, 1, &scissor);
    }

    void Begin(const VkVideoBeginCodingInfoKHR *pBeginInfo)
    {
        BeginVideoCodingKHR(pBeginInfo);
    }

    void End(const VkVideoEndCodingInfoKHR *pEndInfo)
    {
        EndVideoCodingKHR(pEndInfo);
    }

    void PushConstants(VkPipelineLayout pipelineLayout, Shader::Stage stage, uint32_t offset, uint32_t size, const void *data)
    {
        PushConstants(pipelineLayout, (VkShaderStageFlags)stage, offset, size, data);
    }

protected:
    void __Record()
    {
        count++;
    }

    void Swap(CommandBuffer &other)
    {
        Handle::Swap((Handle &)other);
        std::swap(count, other.count);
        std::swap(state, other.state);
        std::swap(level, other.level);
    }

protected:
    CommandPool *commandPool;

    uint32_t count;

    State state;

    VkCommandBufferLevel level;
};

}
}
