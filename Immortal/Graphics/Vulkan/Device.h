#pragma once

#include "Core.h"
#include "Graphics/LightGraphics.h"

#include "Common.h"
#include "Instance.h"
#include "PhysicalDevice.h"
#include "Queue.h"
#include "FencePool.h"
#include "SemaphorePool.h"
#include "Interface/IObject.h"
#include "TimelineCommandBuffer.h"
#include "TimelineCommandPool.h"
#include "Submitter.h"

#include <queue>
#include <future>

namespace Immortal
{
namespace Vulkan
{

class DescriptorPool;
class Swapchain;
class Device : public SuperDevice
{
public:
    static inline Device *That = nullptr;

    void EnableGlobal()
    {
        That = this;
    }

    using Primitive = VkDevice;
    VKCPP_OPERATOR_HANDLE()

public:
    PFN_vkVoidFunction GetProcAddr(char const *pName)
    {
        return vkGetDeviceProcAddr(handle, pName);
    }

    void DestroyDevice(VkAllocationCallbacks const *pAllocator)
    {
        vkDestroyDevice(handle, pAllocator);
    }

    void GetQueue(uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue *pQueue)
    {
        vkGetDeviceQueue(handle, queueFamilyIndex, queueIndex, pQueue);
    }

    VkResult WaitIdle()
    {
        return vkDeviceWaitIdle(handle);
    }

    VkResult AllocateMemory(VkMemoryAllocateInfo const *pAllocateInfo, VkDeviceMemory *pMemory, VkAllocationCallbacks const *pAllocator = nullptr)
    {
        return vkAllocateMemory(handle, pAllocateInfo, pAllocator, pMemory);
    }

    void FreeMemory(VkDeviceMemory memory, VkAllocationCallbacks const *pAllocator)
    {
        vkFreeMemory(handle, memory, pAllocator);
    }

    VkResult MapMemory(VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void **ppData)
    {
        return vkMapMemory(handle, memory, offset, size, flags, ppData);
    }

    void UnmapMemory(VkDeviceMemory memory)
    {
        vkUnmapMemory(handle, memory);
    }

    VkResult FlushMappedMemoryRanges(uint32_t memoryRangeCount, VkMappedMemoryRange const *pMemoryRanges)
    {
        return vkFlushMappedMemoryRanges(handle, memoryRangeCount, pMemoryRanges);
    }

    VkResult InvalidateMappedMemoryRanges(uint32_t memoryRangeCount, VkMappedMemoryRange const *pMemoryRanges)
    {
        return vkInvalidateMappedMemoryRanges(handle, memoryRangeCount, pMemoryRanges);
    }

    void GetMemoryCommitment(VkDeviceMemory memory, VkDeviceSize *pCommittedMemoryInBytes)
    {
        vkGetDeviceMemoryCommitment(handle, memory, pCommittedMemoryInBytes);
    }

    void GetBufferMemoryRequirements(VkBuffer buffer, VkMemoryRequirements *pMemoryRequirements)
    {
        vkGetBufferMemoryRequirements(handle, buffer, pMemoryRequirements);
    }

    VkResult BindBufferMemory(VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize memoryOffset)
    {
        return vkBindBufferMemory(handle, buffer, memory, memoryOffset);
    }

    void GetImageMemoryRequirements(VkImage image, VkMemoryRequirements *pMemoryRequirements)
    {
        vkGetImageMemoryRequirements(handle, image, pMemoryRequirements);
    }

    VkResult BindImageMemory(VkImage image, VkDeviceMemory memory, VkDeviceSize memoryOffset)
    {
        return vkBindImageMemory(handle, image, memory, memoryOffset);
    }

    void GetImageSparseMemoryRequirements(VkImage image, uint32_t *pSparseMemoryRequirementCount, VkSparseImageMemoryRequirements *pSparseMemoryRequirements)
    {
        vkGetImageSparseMemoryRequirements(handle, image, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
    }

    VkResult CreateFence(VkFenceCreateInfo const *pCreateInfo, VkFence *pFence, VkAllocationCallbacks const *pAllocator = nullptr)
    {
        return vkCreateFence(handle, pCreateInfo, pAllocator, pFence);
    }

    void DestroyFence(VkFence fence, VkAllocationCallbacks const *pAllocator)
    {
        vkDestroyFence(handle, fence, pAllocator);
    }

    VkResult ResetFences(uint32_t fenceCount, VkFence const *pFences)
    {
        return vkResetFences(handle, fenceCount, pFences);
    }

    VkResult GetFenceStatus(VkFence fence)
    {
        return vkGetFenceStatus(handle, fence);
    }

    VkResult WaitForFences(uint32_t fenceCount, VkFence const *pFences, VkBool32 waitAll, uint64_t timeout)
    {
        return vkWaitForFences(handle, fenceCount, pFences, waitAll, timeout);
    }

    VkResult CreateSemaphore(VkSemaphoreCreateInfo const *pCreateInfo, VkSemaphore *pSemaphore, VkAllocationCallbacks const *pAllocator = nullptr)
    {
        return vkCreateSemaphore(handle, pCreateInfo, pAllocator, pSemaphore);
    }

    void DestroySemaphore(VkSemaphore semaphore, VkAllocationCallbacks const *pAllocator)
    {
        vkDestroySemaphore(handle, semaphore, pAllocator);
    }

    VkResult CreateEvent(VkEventCreateInfo const *pCreateInfo, VkEvent *pEvent, VkAllocationCallbacks const *pAllocator = nullptr)
    {
        return vkCreateEvent(handle, pCreateInfo, pAllocator, pEvent);
    }

    void DestroyEvent(VkEvent event, VkAllocationCallbacks const *pAllocator)
    {
        vkDestroyEvent(handle, event, pAllocator);
    }

    VkResult GetEventStatus(VkEvent event)
    {
        return vkGetEventStatus(handle, event);
    }

    VkResult SetEvent(VkEvent event)
    {
        return vkSetEvent(handle, event);
    }

    VkResult ResetEvent(VkEvent event)
    {
        return vkResetEvent(handle, event);
    }

    VkResult CreateQueryPool(VkQueryPoolCreateInfo const *pCreateInfo, VkQueryPool *pQueryPool, VkAllocationCallbacks const *pAllocator = nullptr)
    {
        return vkCreateQueryPool(handle, pCreateInfo, pAllocator, pQueryPool);
    }

    void DestroyQueryPool(VkQueryPool queryPool, VkAllocationCallbacks const *pAllocator)
    {
        vkDestroyQueryPool(handle, queryPool, pAllocator);
    }

    VkResult GetQueryPoolResults(VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount, size_t dataSize, void *pData, VkDeviceSize stride, VkQueryResultFlags flags)
    {
        return vkGetQueryPoolResults(handle, queryPool, firstQuery, queryCount, dataSize, pData, stride, flags);
    }

    void ResetQueryPool(VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount)
    {
        vkResetQueryPool(handle, queryPool, firstQuery, queryCount);
    }

    VkResult CreateBuffer(VkBufferCreateInfo const *pCreateInfo, VkBuffer *pBuffer, VkAllocationCallbacks const *pAllocator = nullptr)
    {
        return vkCreateBuffer(handle, pCreateInfo, pAllocator, pBuffer);
    }

    void DestroyBuffer(VkBuffer buffer, VkAllocationCallbacks const *pAllocator)
    {
        vkDestroyBuffer(handle, buffer, pAllocator);
    }

    VkResult CreateBufferView(VkBufferViewCreateInfo const *pCreateInfo, VkBufferView *pView, VkAllocationCallbacks const *pAllocator = nullptr)
    {
        return vkCreateBufferView(handle, pCreateInfo, pAllocator, pView);
    }

    void DestroyBufferView(VkBufferView bufferView, VkAllocationCallbacks const *pAllocator)
    {
        vkDestroyBufferView(handle, bufferView, pAllocator);
    }

    VkResult CreateImage(VkImageCreateInfo const *pCreateInfo, VkImage *pImage, VkAllocationCallbacks const *pAllocator = nullptr)
    {
        return vkCreateImage(handle, pCreateInfo, pAllocator, pImage);
    }

    void DestroyImage(VkImage image, VkAllocationCallbacks const *pAllocator)
    {
        vkDestroyImage(handle, image, pAllocator);
    }

    void GetImageSubresourceLayout(VkImage image, VkImageSubresource const *pSubresource, VkSubresourceLayout *pLayout)
    {
        vkGetImageSubresourceLayout(handle, image, pSubresource, pLayout);
    }

    VkResult CreateImageView(VkImageViewCreateInfo const *pCreateInfo, VkImageView *pView, VkAllocationCallbacks const *pAllocator = nullptr)
    {
        return vkCreateImageView(handle, pCreateInfo, pAllocator, pView);
    }

    void DestroyImageView(VkImageView imageView, VkAllocationCallbacks const *pAllocator)
    {
        vkDestroyImageView(handle, imageView, pAllocator);
    }

    VkResult CreateShaderModule(VkShaderModuleCreateInfo const *pCreateInfo, VkShaderModule *pShaderModule, VkAllocationCallbacks const *pAllocator = nullptr)
    {
        return vkCreateShaderModule(handle, pCreateInfo, pAllocator, pShaderModule);
    }

    void DestroyShaderModule(VkShaderModule shaderModule, VkAllocationCallbacks const *pAllocator)
    {
        vkDestroyShaderModule(handle, shaderModule, pAllocator);
    }

    VkResult CreatePipelineCache(VkPipelineCacheCreateInfo const *pCreateInfo, VkPipelineCache *pPipelineCache, VkAllocationCallbacks const *pAllocator = nullptr)
    {
        return vkCreatePipelineCache(handle, pCreateInfo, pAllocator, pPipelineCache);
    }

    void DestroyPipelineCache(VkPipelineCache pipelineCache, VkAllocationCallbacks const *pAllocator)
    {
        vkDestroyPipelineCache(handle, pipelineCache, pAllocator);
    }

    VkResult GetPipelineCacheData(VkPipelineCache pipelineCache, size_t *pDataSize, void *pData)
    {
        return vkGetPipelineCacheData(handle, pipelineCache, pDataSize, pData);
    }

    VkResult MergePipelineCaches(VkPipelineCache dstCache, uint32_t srcCacheCount, VkPipelineCache const *pSrcCaches)
    {
        return vkMergePipelineCaches(handle, dstCache, srcCacheCount, pSrcCaches);
    }

    VkResult CreateGraphicsPipelines(VkPipelineCache pipelineCache, uint32_t createInfoCount, VkGraphicsPipelineCreateInfo const *pCreateInfos, VkPipeline *pPipelines, VkAllocationCallbacks const *pAllocator = nullptr)
    {
        return vkCreateGraphicsPipelines(handle, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
    }

    VkResult CreateComputePipelines(VkPipelineCache pipelineCache, uint32_t createInfoCount, VkComputePipelineCreateInfo const *pCreateInfos, VkPipeline *pPipelines, VkAllocationCallbacks const *pAllocator = nullptr)
    {
        return vkCreateComputePipelines(handle, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
    }

    VkResult GetSubpassShadingMaxWorkgroupSizeHUAWEI(VkRenderPass renderpass, VkExtent2D *pMaxWorkgroupSize)
    {
        return vkGetDeviceSubpassShadingMaxWorkgroupSizeHUAWEI(handle, renderpass, pMaxWorkgroupSize);
    }

    void DestroyPipeline(VkPipeline pipeline, VkAllocationCallbacks const *pAllocator)
    {
        vkDestroyPipeline(handle, pipeline, pAllocator);
    }

    VkResult CreatePipelineLayout(VkPipelineLayoutCreateInfo const *pCreateInfo, VkPipelineLayout *pPipelineLayout, VkAllocationCallbacks const *pAllocator = nullptr)
    {
        return vkCreatePipelineLayout(handle, pCreateInfo, pAllocator, pPipelineLayout);
    }

    void DestroyPipelineLayout(VkPipelineLayout pipelineLayout, VkAllocationCallbacks const *pAllocator)
    {
        vkDestroyPipelineLayout(handle, pipelineLayout, pAllocator);
    }

    VkResult CreateSampler(VkSamplerCreateInfo const *pCreateInfo, VkSampler *pSampler, VkAllocationCallbacks const *pAllocator = nullptr)
    {
        return vkCreateSampler(handle, pCreateInfo, pAllocator, pSampler);
    }

    void DestroySampler(VkSampler sampler, VkAllocationCallbacks const *pAllocator)
    {
        vkDestroySampler(handle, sampler, pAllocator);
    }

    VkResult CreateDescriptorSetLayout(VkDescriptorSetLayoutCreateInfo const *pCreateInfo, VkDescriptorSetLayout *pSetLayout, VkAllocationCallbacks const *pAllocator = nullptr)
    {
        return vkCreateDescriptorSetLayout(handle, pCreateInfo, pAllocator, pSetLayout);
    }

    void DestroyDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout, VkAllocationCallbacks const *pAllocator)
    {
        vkDestroyDescriptorSetLayout(handle, descriptorSetLayout, pAllocator);
    }

    VkResult CreateDescriptorPool(VkDescriptorPoolCreateInfo const *pCreateInfo, VkDescriptorPool *pDescriptorPool, VkAllocationCallbacks const *pAllocator = nullptr)
    {
        return vkCreateDescriptorPool(handle, pCreateInfo, pAllocator, pDescriptorPool);
    }

    void DestroyDescriptorPool(VkDescriptorPool descriptorPool, VkAllocationCallbacks const *pAllocator)
    {
        vkDestroyDescriptorPool(handle, descriptorPool, pAllocator);
    }

    VkResult ResetDescriptorPool(VkDescriptorPool descriptorPool, VkDescriptorPoolResetFlags flags)
    {
        return vkResetDescriptorPool(handle, descriptorPool, flags);
    }

    VkResult AllocateDescriptorSets(VkDescriptorSetAllocateInfo const *pAllocateInfo, VkDescriptorSet *pDescriptorSets)
    {
        return vkAllocateDescriptorSets(handle, pAllocateInfo, pDescriptorSets);
    }

    VkResult FreeDescriptorSets(VkDescriptorPool descriptorPool, uint32_t descriptorSetCount, VkDescriptorSet const *pDescriptorSets)
    {
        return vkFreeDescriptorSets(handle, descriptorPool, descriptorSetCount, pDescriptorSets);
    }

    void UpdateDescriptorSets(uint32_t descriptorWriteCount, VkWriteDescriptorSet const *pDescriptorWrites, uint32_t descriptorCopyCount, VkCopyDescriptorSet const *pDescriptorCopies)
    {
        vkUpdateDescriptorSets(handle, descriptorWriteCount, pDescriptorWrites, descriptorCopyCount, pDescriptorCopies);
    }

    VkResult CreateFramebuffer(VkFramebufferCreateInfo const *pCreateInfo, VkFramebuffer *pFramebuffer, VkAllocationCallbacks const *pAllocator = nullptr)
    {
        return vkCreateFramebuffer(handle, pCreateInfo, pAllocator, pFramebuffer);
    }

    void DestroyFramebuffer(VkFramebuffer framebuffer, VkAllocationCallbacks const *pAllocator)
    {
        vkDestroyFramebuffer(handle, framebuffer, pAllocator);
    }

    VkResult CreateRenderPass(VkRenderPassCreateInfo const *pCreateInfo, VkRenderPass *pRenderPass, VkAllocationCallbacks const *pAllocator = nullptr)
    {
        return vkCreateRenderPass(handle, pCreateInfo, pAllocator, pRenderPass);
    }

    void DestroyRenderPass(VkRenderPass renderPass, VkAllocationCallbacks const *pAllocator)
    {
        vkDestroyRenderPass(handle, renderPass, pAllocator);
    }

    void GetRenderAreaGranularity(VkRenderPass renderPass, VkExtent2D *pGranularity)
    {
        vkGetRenderAreaGranularity(handle, renderPass, pGranularity);
    }

    VkResult CreateCommandPool(VkCommandPoolCreateInfo const *pCreateInfo, VkCommandPool *pCommandPool, VkAllocationCallbacks const *pAllocator = nullptr)
    {
        return vkCreateCommandPool(handle, pCreateInfo, pAllocator, pCommandPool);
    }

    void DestroyCommandPool(VkCommandPool commandPool, VkAllocationCallbacks const *pAllocator)
    {
        vkDestroyCommandPool(handle, commandPool, pAllocator);
    }

    VkResult ResetCommandPool(VkCommandPool commandPool, VkCommandPoolResetFlags flags)
    {
        return vkResetCommandPool(handle, commandPool, flags);
    }

    VkResult AllocateCommandBuffers(VkCommandBufferAllocateInfo const *pAllocateInfo, VkCommandBuffer *pCommandBuffers)
    {
        return vkAllocateCommandBuffers(handle, pAllocateInfo, pCommandBuffers);
    }

    void FreeCommandBuffers(VkCommandPool commandPool, uint32_t commandBufferCount, VkCommandBuffer const *pCommandBuffers)
    {
        vkFreeCommandBuffers(handle, commandPool, commandBufferCount, pCommandBuffers);
    }

    VkResult CreateSharedSwapchainsKHR(uint32_t swapchainCount, VkSwapchainCreateInfoKHR const *pCreateInfos, VkSwapchainKHR *pSwapchains, VkAllocationCallbacks const *pAllocator = nullptr)
    {
        return vkCreateSharedSwapchainsKHR(handle, swapchainCount, pCreateInfos, pAllocator, pSwapchains);
    }

    VkResult CreateSwapchainKHR(VkSwapchainCreateInfoKHR const *pCreateInfo, VkSwapchainKHR *pSwapchain, VkAllocationCallbacks const *pAllocator = nullptr)
    {
        return vkCreateSwapchainKHR(handle, pCreateInfo, pAllocator, pSwapchain);
    }

    void DestroySwapchainKHR(VkSwapchainKHR swapchain, VkAllocationCallbacks const *pAllocator)
    {
        vkDestroySwapchainKHR(handle, swapchain, pAllocator);
    }

    VkResult GetSwapchainImagesKHR(VkSwapchainKHR swapchain, uint32_t *pSwapchainImageCount, VkImage *pSwapchainImages)
    {
        return vkGetSwapchainImagesKHR(handle, swapchain, pSwapchainImageCount, pSwapchainImages);
    }

    VkResult AcquireNextImageKHR(VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore, VkFence fence, uint32_t *pImageIndex)
    {
        return vkAcquireNextImageKHR(handle, swapchain, timeout, semaphore, fence, pImageIndex);
    }

    VkResult DebugMarkerSetObjectNameEXT(VkDebugMarkerObjectNameInfoEXT const *pNameInfo)
    {
        return vkDebugMarkerSetObjectNameEXT(handle, pNameInfo);
    }

    VkResult DebugMarkerSetObjectTagEXT(VkDebugMarkerObjectTagInfoEXT const *pTagInfo)
    {
        return vkDebugMarkerSetObjectTagEXT(handle, pTagInfo);
    }

    void GetGeneratedCommandsMemoryRequirementsNV(VkGeneratedCommandsMemoryRequirementsInfoNV const *pInfo, VkMemoryRequirements2 *pMemoryRequirements)
    {
        vkGetGeneratedCommandsMemoryRequirementsNV(handle, pInfo, pMemoryRequirements);
    }

    VkResult CreateIndirectCommandsLayoutNV(VkIndirectCommandsLayoutCreateInfoNV const *pCreateInfo, VkIndirectCommandsLayoutNV *pIndirectCommandsLayout, VkAllocationCallbacks const *pAllocator = nullptr)
    {
        return vkCreateIndirectCommandsLayoutNV(handle, pCreateInfo, pAllocator, pIndirectCommandsLayout);
    }

    void DestroyIndirectCommandsLayoutNV(VkIndirectCommandsLayoutNV indirectCommandsLayout, VkAllocationCallbacks const *pAllocator)
    {
        vkDestroyIndirectCommandsLayoutNV(handle, indirectCommandsLayout, pAllocator);
    }

    void TrimCommandPool(VkCommandPool commandPool, VkCommandPoolTrimFlags flags)
    {
        vkTrimCommandPool(handle, commandPool, flags);
    }

    VkResult GetMemoryFdKHR(VkMemoryGetFdInfoKHR const *pGetFdInfo, int *pFd)
    {
        return vkGetMemoryFdKHR(handle, pGetFdInfo, pFd);
    }

    VkResult GetMemoryFdPropertiesKHR(VkExternalMemoryHandleTypeFlagBits handleType, int fd, VkMemoryFdPropertiesKHR *pMemoryFdProperties)
    {
        return vkGetMemoryFdPropertiesKHR(handle, handleType, fd, pMemoryFdProperties);
    }

    VkResult GetMemoryRemoteAddressNV(VkMemoryGetRemoteAddressInfoNV const *pMemoryGetRemoteAddressInfo, VkRemoteAddressNV *pAddress)
    {
        return vkGetMemoryRemoteAddressNV(handle, pMemoryGetRemoteAddressInfo, pAddress);
    }

    VkResult GetSemaphoreFdKHR(VkSemaphoreGetFdInfoKHR const *pGetFdInfo, int *pFd)
    {
        return vkGetSemaphoreFdKHR(handle, pGetFdInfo, pFd);
    }

    VkResult ImportSemaphoreFdKHR(VkImportSemaphoreFdInfoKHR const *pImportSemaphoreFdInfo)
    {
        return vkImportSemaphoreFdKHR(handle, pImportSemaphoreFdInfo);
    }

    VkResult GetFenceFdKHR(VkFenceGetFdInfoKHR const *pGetFdInfo, int *pFd)
    {
        return vkGetFenceFdKHR(handle, pGetFdInfo, pFd);
    }

    VkResult ImportFenceFdKHR(VkImportFenceFdInfoKHR const *pImportFenceFdInfo)
    {
        return vkImportFenceFdKHR(handle, pImportFenceFdInfo);
    }

    VkResult DisplayPowerControlEXT(VkDisplayKHR display, VkDisplayPowerInfoEXT const *pDisplayPowerInfo)
    {
        return vkDisplayPowerControlEXT(handle, display, pDisplayPowerInfo);
    }

    VkResult RegisterEventEXT(VkDeviceEventInfoEXT const *pDeviceEventInfo, VkFence *pFence, VkAllocationCallbacks const *pAllocator = nullptr)
    {
        return vkRegisterDeviceEventEXT(handle, pDeviceEventInfo, pAllocator, pFence);
    }

    VkResult RegisterDisplayEventEXT(VkDisplayKHR display, VkDisplayEventInfoEXT const *pDisplayEventInfo, VkFence *pFence, VkAllocationCallbacks const *pAllocator = nullptr)
    {
        return vkRegisterDisplayEventEXT(handle, display, pDisplayEventInfo, pAllocator, pFence);
    }

    VkResult GetSwapchainCounterEXT(VkSwapchainKHR swapchain, VkSurfaceCounterFlagBitsEXT counter, uint64_t *pCounterValue)
    {
        return vkGetSwapchainCounterEXT(handle, swapchain, counter, pCounterValue);
    }

    void GetGroupPeerMemoryFeatures(uint32_t heapIndex, uint32_t localDeviceIndex, uint32_t remoteDeviceIndex, VkPeerMemoryFeatureFlags *pPeerMemoryFeatures)
    {
        vkGetDeviceGroupPeerMemoryFeatures(handle, heapIndex, localDeviceIndex, remoteDeviceIndex, pPeerMemoryFeatures);
    }

    VkResult BindBufferMemory2(uint32_t bindInfoCount, VkBindBufferMemoryInfo const *pBindInfos)
    {
        return vkBindBufferMemory2(handle, bindInfoCount, pBindInfos);
    }

    VkResult BindImageMemory2(uint32_t bindInfoCount, VkBindImageMemoryInfo const *pBindInfos)
    {
        return vkBindImageMemory2(handle, bindInfoCount, pBindInfos);
    }

    VkResult GetGroupPresentCapabilitiesKHR(VkDeviceGroupPresentCapabilitiesKHR *pDeviceGroupPresentCapabilities)
    {
        return vkGetDeviceGroupPresentCapabilitiesKHR(handle, pDeviceGroupPresentCapabilities);
    }

    VkResult GetGroupSurfacePresentModesKHR(VkSurfaceKHR surface, VkDeviceGroupPresentModeFlagsKHR *pModes)
    {
        return vkGetDeviceGroupSurfacePresentModesKHR(handle, surface, pModes);
    }

    VkResult AcquireNextImage2KHR(VkAcquireNextImageInfoKHR const *pAcquireInfo, uint32_t *pImageIndex)
    {
        return vkAcquireNextImage2KHR(handle, pAcquireInfo, pImageIndex);
    }

    VkResult CreateDescriptorUpdateTemplate(VkDescriptorUpdateTemplateCreateInfo const *pCreateInfo, VkDescriptorUpdateTemplate *pDescriptorUpdateTemplate, VkAllocationCallbacks const *pAllocator = nullptr)
    {
        return vkCreateDescriptorUpdateTemplate(handle, pCreateInfo, pAllocator, pDescriptorUpdateTemplate);
    }

    void DestroyDescriptorUpdateTemplate(VkDescriptorUpdateTemplate descriptorUpdateTemplate, VkAllocationCallbacks const *pAllocator)
    {
        vkDestroyDescriptorUpdateTemplate(handle, descriptorUpdateTemplate, pAllocator);
    }

    void UpdateDescriptorSetWithTemplate(VkDescriptorSet descriptorSet, VkDescriptorUpdateTemplate descriptorUpdateTemplate, void const *pData)
    {
        vkUpdateDescriptorSetWithTemplate(handle, descriptorSet, descriptorUpdateTemplate, pData);
    }

    void SetHdrMetadataEXT(uint32_t swapchainCount, VkSwapchainKHR const *pSwapchains, VkHdrMetadataEXT const *pMetadata)
    {
        vkSetHdrMetadataEXT(handle, swapchainCount, pSwapchains, pMetadata);
    }

    VkResult GetSwapchainStatusKHR(VkSwapchainKHR swapchain)
    {
        return vkGetSwapchainStatusKHR(handle, swapchain);
    }

    VkResult GetRefreshCycleDurationGOOGLE(VkSwapchainKHR swapchain, VkRefreshCycleDurationGOOGLE *pDisplayTimingProperties)
    {
        return vkGetRefreshCycleDurationGOOGLE(handle, swapchain, pDisplayTimingProperties);
    }

    VkResult GetPastPresentationTimingGOOGLE(VkSwapchainKHR swapchain, uint32_t *pPresentationTimingCount, VkPastPresentationTimingGOOGLE *pPresentationTimings)
    {
        return vkGetPastPresentationTimingGOOGLE(handle, swapchain, pPresentationTimingCount, pPresentationTimings);
    }

    void GetBufferMemoryRequirements2(VkBufferMemoryRequirementsInfo2 const *pInfo, VkMemoryRequirements2 *pMemoryRequirements)
    {
        vkGetBufferMemoryRequirements2(handle, pInfo, pMemoryRequirements);
    }

    void GetImageMemoryRequirements2(VkImageMemoryRequirementsInfo2 const *pInfo, VkMemoryRequirements2 *pMemoryRequirements)
    {
        vkGetImageMemoryRequirements2(handle, pInfo, pMemoryRequirements);
    }

    void GetImageSparseMemoryRequirements2(VkImageSparseMemoryRequirementsInfo2 const *pInfo, uint32_t *pSparseMemoryRequirementCount, VkSparseImageMemoryRequirements2 *pSparseMemoryRequirements)
    {
        vkGetImageSparseMemoryRequirements2(handle, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
    }

    void GetBufferMemoryRequirements(VkDeviceBufferMemoryRequirements const *pInfo, VkMemoryRequirements2 *pMemoryRequirements)
    {
        vkGetDeviceBufferMemoryRequirements(handle, pInfo, pMemoryRequirements);
    }

    void GetImageMemoryRequirements(VkDeviceImageMemoryRequirements const *pInfo, VkMemoryRequirements2 *pMemoryRequirements)
    {
        vkGetDeviceImageMemoryRequirements(handle, pInfo, pMemoryRequirements);
    }

    void GetImageSparseMemoryRequirements(VkDeviceImageMemoryRequirements const *pInfo, uint32_t *pSparseMemoryRequirementCount, VkSparseImageMemoryRequirements2 *pSparseMemoryRequirements)
    {
        vkGetDeviceImageSparseMemoryRequirements(handle, pInfo, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
    }

    VkResult CreateSamplerYcbcrConversion(VkSamplerYcbcrConversionCreateInfo const *pCreateInfo, VkSamplerYcbcrConversion *pYcbcrConversion, VkAllocationCallbacks const *pAllocator = nullptr)
    {
        return vkCreateSamplerYcbcrConversion(handle, pCreateInfo, pAllocator, pYcbcrConversion);
    }

    void DestroySamplerYcbcrConversion(VkSamplerYcbcrConversion ycbcrConversion, VkAllocationCallbacks const *pAllocator)
    {
        vkDestroySamplerYcbcrConversion(handle, ycbcrConversion, pAllocator);
    }

    void Get2(VkDeviceQueueInfo2 const *pQueueInfo, VkQueue *pQueue)
    {
        vkGetDeviceQueue2(handle, pQueueInfo, pQueue);
    }

    VkResult CreateValidationCacheEXT(VkValidationCacheCreateInfoEXT const *pCreateInfo, VkValidationCacheEXT *pValidationCache, VkAllocationCallbacks const *pAllocator = nullptr)
    {
        return vkCreateValidationCacheEXT(handle, pCreateInfo, pAllocator, pValidationCache);
    }

    void DestroyValidationCacheEXT(VkValidationCacheEXT validationCache, VkAllocationCallbacks const *pAllocator)
    {
        vkDestroyValidationCacheEXT(handle, validationCache, pAllocator);
    }

    VkResult GetValidationCacheDataEXT(VkValidationCacheEXT validationCache, size_t *pDataSize, void *pData)
    {
        return vkGetValidationCacheDataEXT(handle, validationCache, pDataSize, pData);
    }

    VkResult MergeValidationCachesEXT(VkValidationCacheEXT dstCache, uint32_t srcCacheCount, VkValidationCacheEXT const *pSrcCaches)
    {
        return vkMergeValidationCachesEXT(handle, dstCache, srcCacheCount, pSrcCaches);
    }

    void GetDescriptorSetLayoutSupport(VkDescriptorSetLayoutCreateInfo const *pCreateInfo, VkDescriptorSetLayoutSupport *pSupport)
    {
        vkGetDescriptorSetLayoutSupport(handle, pCreateInfo, pSupport);
    }

    VkResult GetShaderInfoAMD(VkPipeline pipeline, VkShaderStageFlagBits shaderStage, VkShaderInfoTypeAMD infoType, size_t *pInfoSize, void *pInfo)
    {
        return vkGetShaderInfoAMD(handle, pipeline, shaderStage, infoType, pInfoSize, pInfo);
    }

    void SetLocalDimmingAMD(VkSwapchainKHR swapChain, VkBool32 localDimmingEnable)
    {
        vkSetLocalDimmingAMD(handle, swapChain, localDimmingEnable);
    }

    VkResult GetCalibratedTimestampsEXT(uint32_t timestampCount, VkCalibratedTimestampInfoEXT const *pTimestampInfos, uint64_t *pTimestamps, uint64_t *pMaxDeviation)
    {
        return vkGetCalibratedTimestampsEXT(handle, timestampCount, pTimestampInfos, pTimestamps, pMaxDeviation);
    }

    VkResult SetDebugUtilsObjectNameEXT(VkDebugUtilsObjectNameInfoEXT const *pNameInfo)
    {
        return vkSetDebugUtilsObjectNameEXT(handle, pNameInfo);
    }

    VkResult SetDebugUtilsObjectTagEXT(VkDebugUtilsObjectTagInfoEXT const *pTagInfo)
    {
        return vkSetDebugUtilsObjectTagEXT(handle, pTagInfo);
    }

    VkResult GetMemoryHostPointerPropertiesEXT(VkExternalMemoryHandleTypeFlagBits handleType, void const *pHostPointer, VkMemoryHostPointerPropertiesEXT *pMemoryHostPointerProperties)
    {
        return vkGetMemoryHostPointerPropertiesEXT(handle, handleType, pHostPointer, pMemoryHostPointerProperties);
    }

    VkResult CreateRenderPass2(VkRenderPassCreateInfo2 const *pCreateInfo, VkRenderPass *pRenderPass, VkAllocationCallbacks const *pAllocator = nullptr)
    {
        return vkCreateRenderPass2(handle, pCreateInfo, pAllocator, pRenderPass);
    }

    VkResult GetSemaphoreCounterValue(VkSemaphore semaphore, uint64_t *pValue)
    {
        return vkGetSemaphoreCounterValue(handle, semaphore, pValue);
    }

    VkResult WaitSemaphores(VkSemaphoreWaitInfo const *pWaitInfo, uint64_t timeout)
    {
        return vkWaitSemaphores(handle, pWaitInfo, timeout);
    }

    VkResult SignalSemaphore(VkSemaphoreSignalInfo const *pSignalInfo)
    {
        return vkSignalSemaphore(handle, pSignalInfo);
    }

    VkResult CompileDeferredNV(VkPipeline pipeline, uint32_t shader)
    {
        return vkCompileDeferredNV(handle, pipeline, shader);
    }

    VkResult CreateAccelerationStructureNV(VkAccelerationStructureCreateInfoNV const *pCreateInfo, VkAccelerationStructureNV *pAccelerationStructure, VkAllocationCallbacks const *pAllocator = nullptr)
    {
        return vkCreateAccelerationStructureNV(handle, pCreateInfo, pAllocator, pAccelerationStructure);
    }

    void DestroyAccelerationStructureKHR(VkAccelerationStructureKHR accelerationStructure, VkAllocationCallbacks const *pAllocator)
    {
        vkDestroyAccelerationStructureKHR(handle, accelerationStructure, pAllocator);
    }

    void DestroyAccelerationStructureNV(VkAccelerationStructureNV accelerationStructure, VkAllocationCallbacks const *pAllocator)
    {
        vkDestroyAccelerationStructureNV(handle, accelerationStructure, pAllocator);
    }

    void GetAccelerationStructureMemoryRequirementsNV(VkAccelerationStructureMemoryRequirementsInfoNV const *pInfo, VkMemoryRequirements2KHR *pMemoryRequirements)
    {
        vkGetAccelerationStructureMemoryRequirementsNV(handle, pInfo, pMemoryRequirements);
    }

    VkResult BindAccelerationStructureMemoryNV(uint32_t bindInfoCount, VkBindAccelerationStructureMemoryInfoNV const *pBindInfos)
    {
        return vkBindAccelerationStructureMemoryNV(handle, bindInfoCount, pBindInfos);
    }

    VkResult CopyAccelerationStructureKHR(VkDeferredOperationKHR deferredOperation, VkCopyAccelerationStructureInfoKHR const *pInfo)
    {
        return vkCopyAccelerationStructureKHR(handle, deferredOperation, pInfo);
    }

    VkResult CopyAccelerationStructureToMemoryKHR(VkDeferredOperationKHR deferredOperation, VkCopyAccelerationStructureToMemoryInfoKHR const *pInfo)
    {
        return vkCopyAccelerationStructureToMemoryKHR(handle, deferredOperation, pInfo);
    }

    VkResult CopyMemoryToAccelerationStructureKHR(VkDeferredOperationKHR deferredOperation, VkCopyMemoryToAccelerationStructureInfoKHR const *pInfo)
    {
        return vkCopyMemoryToAccelerationStructureKHR(handle, deferredOperation, pInfo);
    }

    VkResult WriteAccelerationStructuresPropertiesKHR(uint32_t accelerationStructureCount, VkAccelerationStructureKHR const *pAccelerationStructures, VkQueryType queryType, size_t dataSize, void *pData, size_t stride)
    {
        return vkWriteAccelerationStructuresPropertiesKHR(handle, accelerationStructureCount, pAccelerationStructures, queryType, dataSize, pData, stride);
    }

    VkResult GetRayTracingShaderGroupHandlesKHR(VkPipeline pipeline, uint32_t firstGroup, uint32_t groupCount, size_t dataSize, void *pData)
    {
        return vkGetRayTracingShaderGroupHandlesKHR(handle, pipeline, firstGroup, groupCount, dataSize, pData);
    }

    VkResult GetRayTracingCaptureReplayShaderGroupHandlesKHR(VkPipeline pipeline, uint32_t firstGroup, uint32_t groupCount, size_t dataSize, void *pData)
    {
        return vkGetRayTracingCaptureReplayShaderGroupHandlesKHR(handle, pipeline, firstGroup, groupCount, dataSize, pData);
    }

    VkResult GetAccelerationStructureHandleNV(VkAccelerationStructureNV accelerationStructure, size_t dataSize, void *pData)
    {
        return vkGetAccelerationStructureHandleNV(handle, accelerationStructure, dataSize, pData);
    }

    VkResult CreateRayTracingPipelinesNV(VkPipelineCache pipelineCache, uint32_t createInfoCount, VkRayTracingPipelineCreateInfoNV const *pCreateInfos, VkPipeline *pPipelines, VkAllocationCallbacks const *pAllocator = nullptr)
    {
        return vkCreateRayTracingPipelinesNV(handle, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
    }

    VkResult CreateRayTracingPipelinesKHR(VkDeferredOperationKHR deferredOperation, VkPipelineCache pipelineCache, uint32_t createInfoCount, VkRayTracingPipelineCreateInfoKHR const *pCreateInfos, VkPipeline *pPipelines, VkAllocationCallbacks const *pAllocator = nullptr)
    {
        return vkCreateRayTracingPipelinesKHR(handle, deferredOperation, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
    }

    void GetAccelerationStructureCompatibilityKHR(VkAccelerationStructureVersionInfoKHR const *pVersionInfo, VkAccelerationStructureCompatibilityKHR *pCompatibility)
    {
        vkGetDeviceAccelerationStructureCompatibilityKHR(handle, pVersionInfo, pCompatibility);
    }

    VkDeviceSize GetRayTracingShaderGroupStackSizeKHR(VkPipeline pipeline, uint32_t group, VkShaderGroupShaderKHR groupShader)
    {
        return vkGetRayTracingShaderGroupStackSizeKHR(handle, pipeline, group, groupShader);
    }

    uint32_t GetImageViewHandleNVX(VkImageViewHandleInfoNVX const *pInfo)
    {
        return vkGetImageViewHandleNVX(handle, pInfo);
    }

    VkResult GetImageViewAddressNVX(VkImageView imageView, VkImageViewAddressPropertiesNVX *pProperties)
    {
        return vkGetImageViewAddressNVX(handle, imageView, pProperties);
    }

    VkResult AcquireProfilingLockKHR(VkAcquireProfilingLockInfoKHR const *pInfo)
    {
        return vkAcquireProfilingLockKHR(handle, pInfo);
    }

    void ReleaseProfilingLockKHR()
    {
        vkReleaseProfilingLockKHR(handle);
    }

    VkResult GetImageDrmFormatModifierPropertiesEXT(VkImage image, VkImageDrmFormatModifierPropertiesEXT *pProperties)
    {
        return vkGetImageDrmFormatModifierPropertiesEXT(handle, image, pProperties);
    }

    uint64_t GetBufferOpaqueCaptureAddress(VkBufferDeviceAddressInfo const *pInfo)
    {
        return vkGetBufferOpaqueCaptureAddress(handle, pInfo);
    }

    VkDeviceAddress GetBufferAddress(VkBufferDeviceAddressInfo const *pInfo)
    {
        return vkGetBufferDeviceAddress(handle, pInfo);
    }

    VkResult InitializePerformanceApiINTEL(VkInitializePerformanceApiInfoINTEL const *pInitializeInfo)
    {
        return vkInitializePerformanceApiINTEL(handle, pInitializeInfo);
    }

    void UninitializePerformanceApiINTEL()
    {
        vkUninitializePerformanceApiINTEL(handle);
    }

    VkResult AcquirePerformanceConfigurationINTEL(VkPerformanceConfigurationAcquireInfoINTEL const *pAcquireInfo, VkPerformanceConfigurationINTEL *pConfiguration)
    {
        return vkAcquirePerformanceConfigurationINTEL(handle, pAcquireInfo, pConfiguration);
    }

    VkResult ReleasePerformanceConfigurationINTEL(VkPerformanceConfigurationINTEL configuration)
    {
        return vkReleasePerformanceConfigurationINTEL(handle, configuration);
    }

    VkResult GetPerformanceParameterINTEL(VkPerformanceParameterTypeINTEL parameter, VkPerformanceValueINTEL *pValue)
    {
        return vkGetPerformanceParameterINTEL(handle, parameter, pValue);
    }

    uint64_t GetMemoryOpaqueCaptureAddress(VkDeviceMemoryOpaqueCaptureAddressInfo const *pInfo)
    {
        return vkGetDeviceMemoryOpaqueCaptureAddress(handle, pInfo);
    }

    VkResult GetPipelineExecutablePropertiesKHR(VkPipelineInfoKHR const *pPipelineInfo, uint32_t *pExecutableCount, VkPipelineExecutablePropertiesKHR *pProperties)
    {
        return vkGetPipelineExecutablePropertiesKHR(handle, pPipelineInfo, pExecutableCount, pProperties);
    }

    VkResult GetPipelineExecutableStatisticsKHR(VkPipelineExecutableInfoKHR const *pExecutableInfo, uint32_t *pStatisticCount, VkPipelineExecutableStatisticKHR *pStatistics)
    {
        return vkGetPipelineExecutableStatisticsKHR(handle, pExecutableInfo, pStatisticCount, pStatistics);
    }

    VkResult GetPipelineExecutableInternalRepresentationsKHR(VkPipelineExecutableInfoKHR const *pExecutableInfo, uint32_t *pInternalRepresentationCount, VkPipelineExecutableInternalRepresentationKHR *pInternalRepresentations)
    {
        return vkGetPipelineExecutableInternalRepresentationsKHR(handle, pExecutableInfo, pInternalRepresentationCount, pInternalRepresentations);
    }

    VkResult CreateAccelerationStructureKHR(VkAccelerationStructureCreateInfoKHR const *pCreateInfo, VkAccelerationStructureKHR *pAccelerationStructure, VkAllocationCallbacks const *pAllocator = nullptr)
    {
        return vkCreateAccelerationStructureKHR(handle, pCreateInfo, pAllocator, pAccelerationStructure);
    }

    VkResult BuildAccelerationStructuresKHR(VkDeferredOperationKHR deferredOperation, uint32_t infoCount, VkAccelerationStructureBuildGeometryInfoKHR const *pInfos, VkAccelerationStructureBuildRangeInfoKHR const * const*ppBuildRangeInfos)
    {
        return vkBuildAccelerationStructuresKHR(handle, deferredOperation, infoCount, pInfos, ppBuildRangeInfos);
    }

    VkDeviceAddress GetAccelerationStructureAddressKHR(VkAccelerationStructureDeviceAddressInfoKHR const *pInfo)
    {
        return vkGetAccelerationStructureDeviceAddressKHR(handle, pInfo);
    }

    VkResult CreateDeferredOperationKHR(VkDeferredOperationKHR *pDeferredOperation, VkAllocationCallbacks const *pAllocator = nullptr)
    {
        return vkCreateDeferredOperationKHR(handle, pAllocator, pDeferredOperation);
    }

    void DestroyDeferredOperationKHR(VkDeferredOperationKHR operation, VkAllocationCallbacks const *pAllocator)
    {
        vkDestroyDeferredOperationKHR(handle, operation, pAllocator);
    }

    uint32_t GetDeferredOperationMaxConcurrencyKHR(VkDeferredOperationKHR operation)
    {
        return vkGetDeferredOperationMaxConcurrencyKHR(handle, operation);
    }

    VkResult GetDeferredOperationResultKHR(VkDeferredOperationKHR operation)
    {
        return vkGetDeferredOperationResultKHR(handle, operation);
    }

    VkResult DeferredOperationJoinKHR(VkDeferredOperationKHR operation)
    {
        return vkDeferredOperationJoinKHR(handle, operation);
    }

    VkResult CreatePrivateDataSlot(VkPrivateDataSlotCreateInfo const *pCreateInfo, VkPrivateDataSlot *pPrivateDataSlot, VkAllocationCallbacks const *pAllocator = nullptr)
    {
        return vkCreatePrivateDataSlot(handle, pCreateInfo, pAllocator, pPrivateDataSlot);
    }

    void DestroyPrivateDataSlot(VkPrivateDataSlot privateDataSlot, VkAllocationCallbacks const *pAllocator)
    {
        vkDestroyPrivateDataSlot(handle, privateDataSlot, pAllocator);
    }

    VkResult SetPrivateData(VkObjectType objectType, uint64_t objectHandle, VkPrivateDataSlot privateDataSlot, uint64_t data)
    {
        return vkSetPrivateData(handle, objectType, objectHandle, privateDataSlot, data);
    }

    void GetPrivateData(VkObjectType objectType, uint64_t objectHandle, VkPrivateDataSlot privateDataSlot, uint64_t *pData)
    {
        vkGetPrivateData(handle, objectType, objectHandle, privateDataSlot, pData);
    }

    void GetAccelerationStructureBuildSizesKHR(VkAccelerationStructureBuildTypeKHR buildType, VkAccelerationStructureBuildGeometryInfoKHR const *pBuildInfo, uint32_t const *pMaxPrimitiveCounts, VkAccelerationStructureBuildSizesInfoKHR *pSizeInfo)
    {
        vkGetAccelerationStructureBuildSizesKHR(handle, buildType, pBuildInfo, pMaxPrimitiveCounts, pSizeInfo);
    }

    VkResult CreateVideoSessionKHR(VkVideoSessionCreateInfoKHR const *pCreateInfo, VkVideoSessionKHR *pVideoSession, VkAllocationCallbacks const *pAllocator = nullptr)
    {
        return vkCreateVideoSessionKHR(handle, pCreateInfo, pAllocator, pVideoSession);
    }

    void DestroyVideoSessionKHR(VkVideoSessionKHR videoSession, VkAllocationCallbacks const *pAllocator)
    {
        vkDestroyVideoSessionKHR(handle, videoSession, pAllocator);
    }

    VkResult CreateVideoSessionParametersKHR(VkVideoSessionParametersCreateInfoKHR const *pCreateInfo, VkVideoSessionParametersKHR *pVideoSessionParameters, VkAllocationCallbacks const *pAllocator = nullptr)
    {
        return vkCreateVideoSessionParametersKHR(handle, pCreateInfo, pAllocator, pVideoSessionParameters);
    }

    VkResult UpdateVideoSessionParametersKHR(VkVideoSessionParametersKHR videoSessionParameters, VkVideoSessionParametersUpdateInfoKHR const *pUpdateInfo)
    {
        return vkUpdateVideoSessionParametersKHR(handle, videoSessionParameters, pUpdateInfo);
    }

    void DestroyVideoSessionParametersKHR(VkVideoSessionParametersKHR videoSessionParameters, VkAllocationCallbacks const *pAllocator)
    {
        vkDestroyVideoSessionParametersKHR(handle, videoSessionParameters, pAllocator);
    }

    VkResult GetVideoSessionMemoryRequirementsKHR(VkVideoSessionKHR videoSession, uint32_t *pMemoryRequirementsCount, VkVideoSessionMemoryRequirementsKHR *pMemoryRequirements)
    {
        return vkGetVideoSessionMemoryRequirementsKHR(handle, videoSession, pMemoryRequirementsCount, pMemoryRequirements);
    }

    VkResult BindVideoSessionMemoryKHR(VkVideoSessionKHR videoSession, uint32_t bindSessionMemoryInfoCount, VkBindVideoSessionMemoryInfoKHR const *pBindSessionMemoryInfos)
    {
        return vkBindVideoSessionMemoryKHR(handle, videoSession, bindSessionMemoryInfoCount, pBindSessionMemoryInfos);
    }

    VkResult CreateCuModuleNVX(VkCuModuleCreateInfoNVX const *pCreateInfo, VkCuModuleNVX *pModule, VkAllocationCallbacks const *pAllocator = nullptr)
    {
        return vkCreateCuModuleNVX(handle, pCreateInfo, pAllocator, pModule);
    }

    VkResult CreateCuFunctionNVX(VkCuFunctionCreateInfoNVX const *pCreateInfo, VkCuFunctionNVX *pFunction, VkAllocationCallbacks const *pAllocator = nullptr)
    {
        return vkCreateCuFunctionNVX(handle, pCreateInfo, pAllocator, pFunction);
    }

    void DestroyCuModuleNVX(VkCuModuleNVX module, VkAllocationCallbacks const *pAllocator)
    {
        vkDestroyCuModuleNVX(handle, module, pAllocator);
    }

    void DestroyCuFunctionNVX(VkCuFunctionNVX function, VkAllocationCallbacks const *pAllocator)
    {
        vkDestroyCuFunctionNVX(handle, function, pAllocator);
    }

    void SetMemoryPriorityEXT(VkDeviceMemory memory, float priority)
    {
        vkSetDeviceMemoryPriorityEXT(handle, memory, priority);
    }

    VkResult WaitForPresentKHR(VkSwapchainKHR swapchain, uint64_t presentId, uint64_t timeout)
    {
        return vkWaitForPresentKHR(handle, swapchain, presentId, timeout);
    }

    void GetDescriptorSetLayoutHostMappingInfoVALVE(VkDescriptorSetBindingReferenceVALVE const *pBindingReference, VkDescriptorSetLayoutHostMappingInfoVALVE *pHostMapping)
    {
        vkGetDescriptorSetLayoutHostMappingInfoVALVE(handle, pBindingReference, pHostMapping);
    }

    void GetDescriptorSetHostMappingVALVE(VkDescriptorSet descriptorSet, void **ppData)
    {
        vkGetDescriptorSetHostMappingVALVE(handle, descriptorSet, ppData);
    }

    VkResult CreateMicromapEXT(VkMicromapCreateInfoEXT const *pCreateInfo, VkMicromapEXT *pMicromap, VkAllocationCallbacks const *pAllocator = nullptr)
    {
        return vkCreateMicromapEXT(handle, pCreateInfo, pAllocator, pMicromap);
    }

    VkResult BuildMicromapsEXT(VkDeferredOperationKHR deferredOperation, uint32_t infoCount, VkMicromapBuildInfoEXT const *pInfos)
    {
        return vkBuildMicromapsEXT(handle, deferredOperation, infoCount, pInfos);
    }

    void DestroyMicromapEXT(VkMicromapEXT micromap, VkAllocationCallbacks const *pAllocator)
    {
        vkDestroyMicromapEXT(handle, micromap, pAllocator);
    }

    VkResult CopyMicromapEXT(VkDeferredOperationKHR deferredOperation, VkCopyMicromapInfoEXT const *pInfo)
    {
        return vkCopyMicromapEXT(handle, deferredOperation, pInfo);
    }

    VkResult CopyMicromapToMemoryEXT(VkDeferredOperationKHR deferredOperation, VkCopyMicromapToMemoryInfoEXT const *pInfo)
    {
        return vkCopyMicromapToMemoryEXT(handle, deferredOperation, pInfo);
    }

    VkResult CopyMemoryToMicromapEXT(VkDeferredOperationKHR deferredOperation, VkCopyMemoryToMicromapInfoEXT const *pInfo)
    {
        return vkCopyMemoryToMicromapEXT(handle, deferredOperation, pInfo);
    }

    VkResult WriteMicromapsPropertiesEXT(uint32_t micromapCount, VkMicromapEXT const *pMicromaps, VkQueryType queryType, size_t dataSize, void *pData, size_t stride)
    {
        return vkWriteMicromapsPropertiesEXT(handle, micromapCount, pMicromaps, queryType, dataSize, pData, stride);
    }

    void GetMicromapCompatibilityEXT(VkMicromapVersionInfoEXT const *pVersionInfo, VkAccelerationStructureCompatibilityKHR *pCompatibility)
    {
        vkGetDeviceMicromapCompatibilityEXT(handle, pVersionInfo, pCompatibility);
    }

    void GetMicromapBuildSizesEXT(VkAccelerationStructureBuildTypeKHR buildType, VkMicromapBuildInfoEXT const *pBuildInfo, VkMicromapBuildSizesInfoEXT *pSizeInfo)
    {
        vkGetMicromapBuildSizesEXT(handle, buildType, pBuildInfo, pSizeInfo);
    }

    void GetShaderModuleIdentifierEXT(VkShaderModule shaderModule, VkShaderModuleIdentifierEXT *pIdentifier)
    {
        vkGetShaderModuleIdentifierEXT(handle, shaderModule, pIdentifier);
    }

    void GetShaderModuleCreateInfoIdentifierEXT(VkShaderModuleCreateInfo const *pCreateInfo, VkShaderModuleIdentifierEXT *pIdentifier)
    {
        vkGetShaderModuleCreateInfoIdentifierEXT(handle, pCreateInfo, pIdentifier);
    }

    void GetImageSubresourceLayout2EXT(VkImage image, VkImageSubresource2EXT const *pSubresource, VkSubresourceLayout2EXT *pLayout)
    {
        vkGetImageSubresourceLayout2EXT(handle, image, pSubresource, pLayout);
    }

    VkResult GetPipelinePropertiesEXT(VkPipelineInfoEXT const *pPipelineInfo, VkBaseOutStructure *pPipelineProperties)
    {
        return vkGetPipelinePropertiesEXT(handle, pPipelineInfo, pPipelineProperties);
    }

    VkResult GetFramebufferTilePropertiesQCOM(VkFramebuffer framebuffer, uint32_t *pPropertiesCount, VkTilePropertiesQCOM *pProperties)
    {
        return vkGetFramebufferTilePropertiesQCOM(handle, framebuffer, pPropertiesCount, pProperties);
    }

    VkResult GetDynamicRenderingTilePropertiesQCOM(VkRenderingInfo const *pRenderingInfo, VkTilePropertiesQCOM *pProperties)
    {
        return vkGetDynamicRenderingTilePropertiesQCOM(handle, pRenderingInfo, pProperties);
    }

    VkResult CreateOpticalFlowSessionNV(VkOpticalFlowSessionCreateInfoNV const *pCreateInfo, VkOpticalFlowSessionNV *pSession, VkAllocationCallbacks const *pAllocator = nullptr)
    {
        return vkCreateOpticalFlowSessionNV(handle, pCreateInfo, pAllocator, pSession);
    }

    void DestroyOpticalFlowSessionNV(VkOpticalFlowSessionNV session, VkAllocationCallbacks const *pAllocator)
    {
        vkDestroyOpticalFlowSessionNV(handle, session, pAllocator);
    }

    VkResult BindOpticalFlowSessionImageNV(VkOpticalFlowSessionNV session, VkOpticalFlowSessionBindingPointNV bindingPoint, VkImageView view, VkImageLayout layout)
    {
        return vkBindOpticalFlowSessionImageNV(handle, session, bindingPoint, view, layout);
    }

    VkResult GetFaultInfoEXT(VkDeviceFaultCountsEXT *pFaultCounts, VkDeviceFaultInfoEXT *pFaultInfo)
    {
        return vkGetDeviceFaultInfoEXT(handle, pFaultCounts, pFaultInfo);
    }

#ifdef _WIN32
    VkResult GetGroupSurfacePresentModes2EXT(VkPhysicalDeviceSurfaceInfo2KHR const *pSurfaceInfo, VkDeviceGroupPresentModeFlagsKHR *pModes)
    {
        return vkGetDeviceGroupSurfacePresentModes2EXT(handle, pSurfaceInfo, pModes);
    }

    VkResult AcquireFullScreenExclusiveModeEXT(VkSwapchainKHR swapchain)
    {
        return vkAcquireFullScreenExclusiveModeEXT(handle, swapchain);
    }

    VkResult ReleaseFullScreenExclusiveModeEXT(VkSwapchainKHR swapchain)
    {
        return vkReleaseFullScreenExclusiveModeEXT(handle, swapchain);
    }

    VkResult GetMemoryWin32HandleNV(VkDeviceMemory memory, VkExternalMemoryHandleTypeFlagsNV handleType, HANDLE *pHandle)
    {
        return vkGetMemoryWin32HandleNV(handle, memory, handleType, pHandle);
    }

    VkResult GetMemoryWin32HandleKHR(VkMemoryGetWin32HandleInfoKHR const *pGetWin32HandleInfo, HANDLE *pHandle)
    {
        return vkGetMemoryWin32HandleKHR(handle, pGetWin32HandleInfo, pHandle);
    }

    VkResult GetMemoryWin32HandlePropertiesKHR(VkExternalMemoryHandleTypeFlagBits handleType, HANDLE hwnd, VkMemoryWin32HandlePropertiesKHR *pMemoryWin32HandleProperties)
    {
        return vkGetMemoryWin32HandlePropertiesKHR(handle, handleType, hwnd, pMemoryWin32HandleProperties);
    }

    VkResult GetSemaphoreWin32HandleKHR(VkSemaphoreGetWin32HandleInfoKHR const *pGetWin32HandleInfo, HANDLE *pHandle)
    {
        return vkGetSemaphoreWin32HandleKHR(handle, pGetWin32HandleInfo, pHandle);
    }

    VkResult ImportSemaphoreWin32HandleKHR(VkImportSemaphoreWin32HandleInfoKHR const *pImportSemaphoreWin32HandleInfo)
    {
        return vkImportSemaphoreWin32HandleKHR(handle, pImportSemaphoreWin32HandleInfo);
    }

    VkResult GetFenceWin32HandleKHR(VkFenceGetWin32HandleInfoKHR const *pGetWin32HandleInfo, HANDLE *pHandle)
    {
        return vkGetFenceWin32HandleKHR(handle, pGetWin32HandleInfo, pHandle);
    }

    VkResult ImportFenceWin32HandleKHR(VkImportFenceWin32HandleInfoKHR const *pImportFenceWin32HandleInfo)
    {
        return vkImportFenceWin32HandleKHR(handle, pImportFenceWin32HandleInfo);
    }
#endif

public:
    Device();

    Device(PhysicalDevice *physicalDevice, std::unordered_map<const char *, bool> requestedExtensions = {});

    virtual ~Device() override;

    virtual Anonymous GetBackendHandle() const override;

    virtual BackendAPI GetBackendAPI() override;

    virtual SuperQueue *CreateQueue(QueueType type, QueuePriority priority) override;

	virtual SuperCommandBuffer *CreateCommandBuffer(QueueType type) override;

	virtual SuperSwapchain *CreateSwapchain(SuperQueue *queue, Window *window, Format format, uint32_t bufferCount, SwapchainMode mode) override;

	virtual SuperSampler *CreateSampler(Filter filter, AddressMode addressMode, CompareOperation compareOperation, float minLod, float maxLod) override;

	virtual SuperShader *CreateShader(const std::string &name, ShaderStage stage, const std::string &source, const std::string &entryPoint) override;

	virtual SuperGraphicsPipeline *CreateGraphicsPipeline() override;

	virtual SuperTexture *CreateTexture(Format format, uint32_t width, uint32_t height, uint16_t mipLevels, uint16_t arrayLayers, TextureType type) override;

	virtual SuperBuffer *CreateBuffer(size_t size, BufferType type) override;

	virtual SuperDescriptorSet *CreateDescriptorSet(SuperPipeline *pipeline) override;

	virtual SuperGPUEvent *CreateGPUEvent(const std::string &name) override;

    virtual SuperRenderTarget *CreateRenderTarget(uint32_t width, uint32_t height, const Format *pColorAttachmentFormats, uint32_t colorAttachmentCount, Format depthAttachmentFormat = {}) override;

public:
    uint32_t GetQueueFailyIndex(VkQueueFlagBits queueFlag);

    uint32_t GetMemoryType(uint32_t bits, VkMemoryPropertyFlags properties, VkBool32 *memoryTypeFound = nullptr);

    void DestroyObjects();

public:
    template <class T>
    void DestroyAsync(T task)
    {
        auto wrapper = std::make_shared<std::packaged_task<decltype(task())()>>(std::move(task));
        {
            std::unique_lock<std::mutex> lock{ destroyCoroutine.mutex };
            destroyCoroutine.queues[destroyCoroutine.working].push([=, this]() -> void {
                (*wrapper)();
                });
        }
    }

    VkFormat DepthFormat(bool depthOnly = false)
    {
        return physicalDevice->GetSuitableDepthFormat(depthOnly);
    }

    bool IsEnabled(const std::string &extension) const
    {
        return std::find_if(enabledExtensions.begin(), enabledExtensions.end(), [extension](const std::string &enabledExtension)
        {
            return Equals(extension.c_str(), enabledExtension.c_str());
        }) != enabledExtensions.end();
    }

    bool IsExtensionSupported(const std::string &extension) const
    {
        return deviceExtensions.find(extension) != deviceExtensions.end();
    }

public:
#define DEFINE_CREATE_VK_OBJECT(T) \
    VkResult Create(const Vk##T##CreateInfo *pCreateInfo, Vk##T *pObject, VkAllocationCallbacks *pAllocator = nullptr) \
    { \
        return vkCreate##T(handle, pCreateInfo, pAllocator, pObject); \
    }

#define DEFINE_CREATE_VK_KHR_OBJECT(T) \
    VkResult Create(const Vk##T##CreateInfoKHR *pCreateInfo, Vk##T##KHR *pObject, VkAllocationCallbacks *pAllocator = nullptr) \
    { \
        return vkCreate##T##KHR(handle, pCreateInfo, pAllocator, pObject); \
    }

#define DEFINE_DESTORY_VK_OBJECT(T) \
    void Destroy(Vk##T object, const VkAllocationCallbacks *pAllocator = nullptr) \
    { \
        if (object != VK_NULL_HANDLE) \
        { \
            vkDeviceWaitIdle(handle); \
            vkDestroy##T(handle, object, pAllocator); \
        } \
    } \
    void DestroyAsync(Vk##T object, const VkAllocationCallbacks *pAllocator = nullptr) \
    { \
        if (object != VK_NULL_HANDLE) \
        { \
            vkDestroy##T(handle, object, pAllocator); \
        } \
    }

#define DEFINE_RESET_OBJECT(T) \
    void Reset(Vk##T object, Vk##T##ResetFlags flags) \
    { \
        Check(vkReset##T(handle, object, flags)); \
    }

#define DEFINE_CREATE_PIPELINES(T) \
    VkResult CreatePipelines(VkPipelineCache pipelineCache, uint32_t createInfoCount, const Vk##T##PipelineCreateInfo *pCreateInfos, VkPipeline *pPipelines, const VkAllocationCallbacks *pAllocator = nullptr) \
    { \
        return vkCreate##T##Pipelines(handle, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines); \
    }

#define DEFINE_GET_REQUIREMENTS(T) \
    void GetRequirements(Vk##T object, VkMemoryRequirements *pMemoryRequirements) \
    { \
        vkGet##T##MemoryRequirements(handle, object, pMemoryRequirements); \
    }

#define DEFINE_BIND_MEMORY(T) \
    void BindMemory(Vk##T buffer, VkDeviceMemory memory, VkDeviceSize memoryOffset) \
    { \
        Check(vkBind##T##Memory(handle, buffer, memory, memoryOffset)); \
    } \
    void BindMemory(const VkBind##T##MemoryInfo *bindInfos, uint32_t count = 1) \
    { \
        Check(vkBind##T##Memory2(handle, count, bindInfos)); \
    }

    DEFINE_CREATE_VK_OBJECT(Buffer)
    DEFINE_CREATE_VK_OBJECT(CommandPool)
    DEFINE_CREATE_VK_OBJECT(DescriptorPool)
    DEFINE_CREATE_VK_OBJECT(DescriptorSetLayout)
    DEFINE_CREATE_VK_OBJECT(Fence)
    DEFINE_CREATE_VK_OBJECT(Framebuffer)
    DEFINE_CREATE_VK_OBJECT(Image)
    DEFINE_CREATE_VK_OBJECT(ImageView)
    DEFINE_CREATE_VK_OBJECT(PipelineLayout)
    DEFINE_CREATE_VK_OBJECT(RenderPass)
    DEFINE_CREATE_VK_OBJECT(Sampler)
    DEFINE_CREATE_VK_OBJECT(Semaphore)
    DEFINE_CREATE_VK_OBJECT(ShaderModule)
    DEFINE_CREATE_VK_OBJECT(PipelineCache)

    DEFINE_CREATE_VK_KHR_OBJECT(Swapchain)
    DEFINE_CREATE_VK_KHR_OBJECT(VideoSession)
    DEFINE_CREATE_VK_KHR_OBJECT(AccelerationStructure)

    DEFINE_DESTORY_VK_OBJECT(AccelerationStructureKHR)
    DEFINE_DESTORY_VK_OBJECT(Buffer)
    DEFINE_DESTORY_VK_OBJECT(CommandPool)
    DEFINE_DESTORY_VK_OBJECT(DescriptorPool)
    DEFINE_DESTORY_VK_OBJECT(DescriptorSetLayout)
    DEFINE_DESTORY_VK_OBJECT(Fence)
    DEFINE_DESTORY_VK_OBJECT(Framebuffer)
    DEFINE_DESTORY_VK_OBJECT(Image)
    DEFINE_DESTORY_VK_OBJECT(ImageView)
    DEFINE_DESTORY_VK_OBJECT(Pipeline)
    DEFINE_DESTORY_VK_OBJECT(PipelineLayout)
    DEFINE_DESTORY_VK_OBJECT(RenderPass)
    DEFINE_DESTORY_VK_OBJECT(Sampler)
    DEFINE_DESTORY_VK_OBJECT(Semaphore)
    DEFINE_DESTORY_VK_OBJECT(ShaderModule)
    DEFINE_DESTORY_VK_OBJECT(SwapchainKHR)
    DEFINE_DESTORY_VK_OBJECT(PipelineCache)

    DEFINE_RESET_OBJECT(CommandPool)
    DEFINE_RESET_OBJECT(DescriptorPool)

    DEFINE_CREATE_PIPELINES(Graphics)
    DEFINE_CREATE_PIPELINES(Compute)

    DEFINE_GET_REQUIREMENTS(Buffer)
    DEFINE_GET_REQUIREMENTS(Image)

    DEFINE_BIND_MEMORY(Buffer)
    DEFINE_BIND_MEMORY(Image)

#define DEFINE_VMA_CREATE_DESTROY_OBJECT(T) \
    VkResult Create##T(const Vk##T##CreateInfo *pCreateInfo, const VmaAllocationCreateInfo *pAllocationCreateInfo, Vk##T *pObject,VmaAllocation *pAllocation, VmaAllocationInfo *pAllocationInfo = nullptr) \
    { \
        return vmaCreate##T(memoryAllocator, pCreateInfo, pAllocationCreateInfo, pObject, pAllocation, pAllocationInfo); \
    } \
    VkResult Create(const Vk##T##CreateInfo *pCreateInfo, const VmaAllocationCreateInfo *pAllocationCreateInfo, Vk##T *pObject, VmaAllocation *pAllocation, VmaAllocationInfo *pAllocationInfo = nullptr) \
    { \
        return vmaCreate##T(memoryAllocator, pCreateInfo, pAllocationCreateInfo, pObject, pAllocation, pAllocationInfo); \
    } \
    void Destroy##T(Vk##T object, VmaAllocation allocation) \
    { \
        vmaDestroy##T(memoryAllocator, object, allocation); \
    } \
    void Destroy(Vk##T object, VmaAllocation allocation) \
    { \
        vmaDestroy##T(memoryAllocator, object, allocation); \
    }

    DEFINE_VMA_CREATE_DESTROY_OBJECT(Buffer)
    DEFINE_VMA_CREATE_DESTROY_OBJECT(Image)

public:
    template <class T>
    T &Get()
    {
        if constexpr (IsPrimitiveOf<PhysicalDevice, T>())
        {
            return *physicalDevice;
        }
        if constexpr (IsPrimitiveOf<Queue, T>())
        {
            return queues;
        }
        if constexpr (IsPrimitiveOf<CommandPool, T>())
        {
            return *commandPool;
        }
    }

    VkResult AllocateDescriptorSet(const VkDescriptorSetLayout *pDescriptorSetLayout, VkDescriptorSet *pDescriptorSets);

    void FreeDescriptorSet(VkDescriptorSet *pDescriptorSets, uint32_t size = 1);

    void FreeDescriptorSet(VkDescriptorSet descriptorSet)
    {
        FreeDescriptorSet(&descriptorSet, 1);
    }

    VkResult Wait()
    {
        return vkDeviceWaitIdle(handle);
    }

    VkResult Wait(const VkFence *pFences, uint32_t fenceCount = 1, VkBool32 waitAll = VK_TRUE, uint64_t timeout = std::numeric_limits<uint64_t>::max()) const
    {
        return vkWaitForFences(handle, fenceCount, pFences, waitAll, timeout);
    }

    VkResult Wait(const VkSemaphore *pSemaphore, uint64_t *pValue, uint32_t semaphoreCount = 1, uint64_t timeout = std::numeric_limits<uint64_t>::max()) const
    {
        VkSemaphoreWaitInfo waitInfo{};
        waitInfo.sType          = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
        waitInfo.pNext          = nullptr;
        waitInfo.flags          = 0;
        waitInfo.semaphoreCount = semaphoreCount;
        waitInfo.pValues        = pValue;
        waitInfo.pSemaphores    = pSemaphore;

        return vkWaitSemaphoresKHR(*this, &waitInfo, timeout);
    }

    VkResult Signal(VkSemaphore semaphore, uint64_t value) const
    {
        VkSemaphoreSignalInfo signalInfo{};
        signalInfo.sType     = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO;
        signalInfo.pNext     = nullptr;
        signalInfo.semaphore = semaphore;
        signalInfo.value     = value;

        return vkSignalSemaphoreKHR(*this, &signalInfo);
    }

    VkResult GetCompletion(VkSemaphore semaphore, uint64_t *pValue) const
    {
        return vkGetSemaphoreCounterValueKHR(*this, semaphore, pValue);
    }

    VkResult Reset(VkFence *pFence, uint32_t fenceCount = 1)
    {
        return vkResetFences(handle, 1, pFence);
    }

    void WaitAndReset(VkFence *pFence, uint32_t fenceCount = 1)
    {
        Check(Wait(pFence, fenceCount));
        Check(Reset(pFence, fenceCount));
    }

    VkDeviceAddress GetBufferAddress(VkBuffer buffer)
    {
        VkBufferDeviceAddressInfo bufferInfo{
            .sType  = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
            .buffer = buffer
        };

        return GetBufferAddress(&bufferInfo);
    }

    VmaAllocator MemoryAllocator() const
    {
        return memoryAllocator;
    }

    VkResult MapMemory(VmaAllocation memory, void **ppData)
    {
		return vmaMapMemory(memoryAllocator, memory, ppData);
    }

    void UnmapMemory(VmaAllocation memory)
    {
		vmaUnmapMemory(memoryAllocator, memory);
    }

    VkResult GetSurfaceCapabilities(VkSurfaceKHR surface, VkSurfaceCapabilitiesKHR *properties)
    {
        return physicalDevice->GetSurfaceCapabilitiesKHR(surface, properties);
    }

    template <class T>
    requires std::derived_from<T, Semaphore>
    VkResult AllocateSemaphore(T *pSemaphore)
    {
        if constexpr (std::is_same_v<T, TimelineSemaphore>)
        {
            *pSemaphore = std::move(semaphorePool->Allocate(0));
        }
        else
        {
            *pSemaphore = semaphorePool->Allocate();
        }
        return VK_SUCCESS;
    }

    template <class T>
    requires std::derived_from<T, Semaphore>
    void Release(T &&semaphore)
    {
        if constexpr (std::is_same_v<T, TimelineSemaphore>)
        {
            semaphorePool->Release(std::move(semaphore));
        }
        else
        {
            semaphorePool->Release(semaphore);
        }
    }

protected:
    PhysicalDevice *physicalDevice;

    VkAllocationCallbacks allocationCallbacks;

    VmaAllocator memoryAllocator;

    std::unordered_set<std::string> deviceExtensions;

    std::vector<const char *> enabledExtensions;

    std::vector<std::vector<DeviceQueue>> queues;

    URef<SemaphorePool> semaphorePool;

    URef<DescriptorPool> descriptorPool;

    URef<CommandPool> commandPool;

    bool isSubmitted = false;

    struct {
        std::array<std::queue<std::function<void()>>, 6> queues;
        std::mutex mutex;
        uint32_t working = 0;
        uint32_t freeing = 1;
    } destroyCoroutine;
};
}
}
