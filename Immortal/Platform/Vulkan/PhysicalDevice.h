#pragma once

#include "Core.h"
#include "Common.h"

#include "Instance.h"
#include <set>

namespace Immortal
{
namespace Vulkan
{
class PhysicalDevice
{
public:
    enum Feature : int
    {
        RobustBufferAccess = 0,
        FullDrawIndexUint32,
        ImageCubeArray,
        IndependentBlend,
        GeometryShader,
        TessellationShader,
        SampleRateShading,
        DualSrcBlend,
        logicOp,
        MultiDrawIndirect,
        DrawIndirectFirstInstance,
        DepthClamp,
        DepthBiasClamp,
        FillModeNonSolid,
        DepthBounds,
        WideLines,
        LargePoints,
        AlphaToOne,
        MultiViewport,
        SamplerAnisotropy,
        TextureCompressionETC2,
        TextureCompressionASTC_LDR,
        TextureCompressionBC,
        OcclusionQueryPrecise,
        PipelineStatisticsQuery,
        VertexPipelineStoresAndAtomics,
        FragmentStoresAndAtomics,
        ShaderTessellationAndGeometryPointSize,
        ShaderImageGatherExtended,
        ShaderStorageImageExtendedFormats,
        ShaderStorageImageMultisample,
        ShaderStorageImageReadWithoutFormat,
        ShaderStorageImageWriteWithoutFormat,
        ShaderUniformBufferArrayDynamicIndexing,
        ShaderSampledImageArrayDynamicIndexing,
        ShaderStorageBufferArrayDynamicIndexing,
        ShaderStorageImageArrayDynamicIndexing,
        ShaderClipDistance,
        ShaderCullDistance,
        ShaderFloat64,
        ShaderInt64,
        ShaderInt16,
        ShaderResourceResidency,
        ShaderResourceMinLod,
        SparseBinding,
        SparseResidencyBuffer,
        SparseResidencyImage2D,
        SparseResidencyImage3D,
        SparseResidency2Samples,
        SparseResidency4Samples,
        SparseResidency8Samples,
        SparseResidency16Samples,
        SparseResidencyAliased,
        VariableMultisampleRate,
        InheritedQueries,
    };

    using Primitive = VkPhysicalDevice;
    VKCPP_OPERATOR_HANDLE()

public:
    void GetProperties(VkPhysicalDeviceProperties *pProperties)
    {
        vkGetPhysicalDeviceProperties(handle, pProperties);
    }

    void GetQueueFamilyProperties(uint32_t *pQueueFamilyPropertyCount, VkQueueFamilyProperties *pQueueFamilyProperties)
    {
        vkGetPhysicalDeviceQueueFamilyProperties(handle, pQueueFamilyPropertyCount, pQueueFamilyProperties);
    }

    void GetMemoryProperties(VkPhysicalDeviceMemoryProperties *pMemoryProperties)
    {
        vkGetPhysicalDeviceMemoryProperties(handle, pMemoryProperties);
    }

    void GetFeatures(VkPhysicalDeviceFeatures *pFeatures)
    {
        vkGetPhysicalDeviceFeatures(handle, pFeatures);
    }

    void GetFormatProperties(VkFormat format, VkFormatProperties *pFormatProperties)
    {
        vkGetPhysicalDeviceFormatProperties(handle, format, pFormatProperties);
    }

    VkResult GetImageFormatProperties(VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags, VkImageFormatProperties *pImageFormatProperties)
    {
        return vkGetPhysicalDeviceImageFormatProperties(handle, format, type, tiling, usage, flags, pImageFormatProperties);
    }

    VkResult CreateDevice(VkDeviceCreateInfo const *pCreateInfo, VkDevice *pDevice, VkAllocationCallbacks const *pAllocator = nullptr)
    {
        return vkCreateDevice(handle, pCreateInfo, pAllocator, pDevice);
    }

    VkResult EnumerateLayerProperties(uint32_t *pPropertyCount, VkLayerProperties *pProperties)
    {
        return vkEnumerateDeviceLayerProperties(handle, pPropertyCount, pProperties);
    }

    VkResult EnumerateDeviceExtensionProperties(char const *pLayerName, uint32_t *pPropertyCount, VkExtensionProperties *pProperties)
    {
        return vkEnumerateDeviceExtensionProperties(handle, pLayerName, pPropertyCount, pProperties);
    }

    void GetSparseImageFormatProperties(VkFormat format, VkImageType type, VkSampleCountFlagBits samples, VkImageUsageFlags usage, VkImageTiling tiling, uint32_t *pPropertyCount, VkSparseImageFormatProperties *pProperties)
    {
        vkGetPhysicalDeviceSparseImageFormatProperties(handle, format, type, samples, usage, tiling, pPropertyCount, pProperties);
    }

    VkResult GetDisplayPropertiesKHR(uint32_t *pPropertyCount, VkDisplayPropertiesKHR *pProperties)
    {
        return vkGetPhysicalDeviceDisplayPropertiesKHR(handle, pPropertyCount, pProperties);
    }

    VkResult GetDisplayPlanePropertiesKHR(uint32_t *pPropertyCount, VkDisplayPlanePropertiesKHR *pProperties)
    {
        return vkGetPhysicalDeviceDisplayPlanePropertiesKHR(handle, pPropertyCount, pProperties);
    }

    VkResult GetDisplayPlaneSupportedDisplaysKHR(uint32_t planeIndex, uint32_t *pDisplayCount, VkDisplayKHR *pDisplays)
    {
        return vkGetDisplayPlaneSupportedDisplaysKHR(handle, planeIndex, pDisplayCount, pDisplays);
    }

    VkResult GetDisplayModePropertiesKHR(VkDisplayKHR display, uint32_t *pPropertyCount, VkDisplayModePropertiesKHR *pProperties)
    {
        return vkGetDisplayModePropertiesKHR(handle, display, pPropertyCount, pProperties);
    }

    VkResult CreateDisplayModeKHR(VkDisplayKHR display, VkDisplayModeCreateInfoKHR const *pCreateInfo, VkDisplayModeKHR *pMode, VkAllocationCallbacks const *pAllocator = nullptr)
    {
        return vkCreateDisplayModeKHR(handle, display, pCreateInfo, pAllocator, pMode);
    }

    VkResult GetDisplayPlaneCapabilitiesKHR(VkDisplayModeKHR mode, uint32_t planeIndex, VkDisplayPlaneCapabilitiesKHR *pCapabilities)
    {
        return vkGetDisplayPlaneCapabilitiesKHR(handle, mode, planeIndex, pCapabilities);
    }

    VkResult GetSurfaceSupportKHR(uint32_t queueFamilyIndex, VkSurfaceKHR surface, VkBool32 *pSupported)
    {
        return vkGetPhysicalDeviceSurfaceSupportKHR(handle, queueFamilyIndex, surface, pSupported);
    }

    VkResult GetSurfaceCapabilitiesKHR(VkSurfaceKHR surface, VkSurfaceCapabilitiesKHR *pSurfaceCapabilities)
    {
        return vkGetPhysicalDeviceSurfaceCapabilitiesKHR(handle, surface, pSurfaceCapabilities);
    }

    VkResult GetSurfaceFormatsKHR(VkSurfaceKHR surface, uint32_t *pSurfaceFormatCount, VkSurfaceFormatKHR *pSurfaceFormats)
    {
        return vkGetPhysicalDeviceSurfaceFormatsKHR(handle, surface, pSurfaceFormatCount, pSurfaceFormats);
    }

    VkResult GetSurfacePresentModesKHR(VkSurfaceKHR surface, uint32_t *pPresentModeCount, VkPresentModeKHR *pPresentModes)
    {
        return vkGetPhysicalDeviceSurfacePresentModesKHR(handle, surface, pPresentModeCount, pPresentModes);
    }

    VkResult GetExternalImageFormatPropertiesNV(VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags, VkExternalMemoryHandleTypeFlagsNV externalHandleType, VkExternalImageFormatPropertiesNV *pExternalImageFormatProperties)
    {
        return vkGetPhysicalDeviceExternalImageFormatPropertiesNV(handle, format, type, tiling, usage, flags, externalHandleType, pExternalImageFormatProperties);
    }

    void GetFeatures2(VkPhysicalDeviceFeatures2 *pFeatures)
    {
        vkGetPhysicalDeviceFeatures2(handle, pFeatures);
    }

    void GetProperties2(VkPhysicalDeviceProperties2 *pProperties)
    {
        vkGetPhysicalDeviceProperties2(handle, pProperties);
    }

    void GetFormatProperties2(VkFormat format, VkFormatProperties2 *pFormatProperties)
    {
        vkGetPhysicalDeviceFormatProperties2(handle, format, pFormatProperties);
    }

    VkResult GetImageFormatProperties2(VkPhysicalDeviceImageFormatInfo2 const *pImageFormatInfo, VkImageFormatProperties2 *pImageFormatProperties)
    {
        return vkGetPhysicalDeviceImageFormatProperties2(handle, pImageFormatInfo, pImageFormatProperties);
    }

    void GetFamilyProperties2(uint32_t *pQueueFamilyPropertyCount, VkQueueFamilyProperties2 *pQueueFamilyProperties)
    {
        vkGetPhysicalDeviceQueueFamilyProperties2(handle, pQueueFamilyPropertyCount, pQueueFamilyProperties);
    }

    void GetMemoryProperties2(VkPhysicalDeviceMemoryProperties2 *pMemoryProperties)
    {
        vkGetPhysicalDeviceMemoryProperties2(handle, pMemoryProperties);
    }

    void GetSparseImageFormatProperties2(VkPhysicalDeviceSparseImageFormatInfo2 const *pFormatInfo, uint32_t *pPropertyCount, VkSparseImageFormatProperties2 *pProperties)
    {
        vkGetPhysicalDeviceSparseImageFormatProperties2(handle, pFormatInfo, pPropertyCount, pProperties);
    }

    void GetExternalBufferProperties(VkPhysicalDeviceExternalBufferInfo const *pExternalBufferInfo, VkExternalBufferProperties *pExternalBufferProperties)
    {
        vkGetPhysicalDeviceExternalBufferProperties(handle, pExternalBufferInfo, pExternalBufferProperties);
    }

    void GetExternalSemaphoreProperties(VkPhysicalDeviceExternalSemaphoreInfo const *pExternalSemaphoreInfo, VkExternalSemaphoreProperties *pExternalSemaphoreProperties)
    {
        vkGetPhysicalDeviceExternalSemaphoreProperties(handle, pExternalSemaphoreInfo, pExternalSemaphoreProperties);
    }

    void GetExternalFenceProperties(VkPhysicalDeviceExternalFenceInfo const *pExternalFenceInfo, VkExternalFenceProperties *pExternalFenceProperties)
    {
        vkGetPhysicalDeviceExternalFenceProperties(handle, pExternalFenceInfo, pExternalFenceProperties);
    }

    VkResult ReleaseDisplayEXT(VkDisplayKHR display)
    {
        return vkReleaseDisplayEXT(handle, display);
    }

    VkResult AcquireWinrtDisplayNV(VkDisplayKHR display)
    {
        return vkAcquireWinrtDisplayNV(handle, display);
    }

    VkResult GetWinrtDisplayNV(uint32_t deviceRelativeId, VkDisplayKHR *pDisplay)
    {
        return vkGetWinrtDisplayNV(handle, deviceRelativeId, pDisplay);
    }

    VkResult GetSurfaceCapabilities2EXT(VkSurfaceKHR surface, VkSurfaceCapabilities2EXT *pSurfaceCapabilities)
    {
        return vkGetPhysicalDeviceSurfaceCapabilities2EXT(handle, surface, pSurfaceCapabilities);
    }

    VkResult GetPresentRectanglesKHR(VkSurfaceKHR surface, uint32_t *pRectCount, VkRect2D *pRects)
    {
        return vkGetPhysicalDevicePresentRectanglesKHR(handle, surface, pRectCount, pRects);
    }

    void GetMultisamplePropertiesEXT(VkSampleCountFlagBits samples, VkMultisamplePropertiesEXT *pMultisampleProperties)
    {
        vkGetPhysicalDeviceMultisamplePropertiesEXT(handle, samples, pMultisampleProperties);
    }

    VkResult GetSurfaceCapabilities2KHR(VkPhysicalDeviceSurfaceInfo2KHR const *pSurfaceInfo, VkSurfaceCapabilities2KHR *pSurfaceCapabilities)
    {
        return vkGetPhysicalDeviceSurfaceCapabilities2KHR(handle, pSurfaceInfo, pSurfaceCapabilities);
    }

    VkResult GetSurfaceFormats2KHR(VkPhysicalDeviceSurfaceInfo2KHR const *pSurfaceInfo, uint32_t *pSurfaceFormatCount, VkSurfaceFormat2KHR *pSurfaceFormats)
    {
        return vkGetPhysicalDeviceSurfaceFormats2KHR(handle, pSurfaceInfo, pSurfaceFormatCount, pSurfaceFormats);
    }

    VkResult GetDisplayProperties2KHR(uint32_t *pPropertyCount, VkDisplayProperties2KHR *pProperties)
    {
        return vkGetPhysicalDeviceDisplayProperties2KHR(handle, pPropertyCount, pProperties);
    }

    VkResult GetDisplayPlaneProperties2KHR(uint32_t *pPropertyCount, VkDisplayPlaneProperties2KHR *pProperties)
    {
        return vkGetPhysicalDeviceDisplayPlaneProperties2KHR(handle, pPropertyCount, pProperties);
    }

    VkResult GetDisplayModeProperties2KHR(VkDisplayKHR display, uint32_t *pPropertyCount, VkDisplayModeProperties2KHR *pProperties)
    {
        return vkGetDisplayModeProperties2KHR(handle, display, pPropertyCount, pProperties);
    }

    VkResult GetDisplayPlaneCapabilities2KHR(VkDisplayPlaneInfo2KHR const *pDisplayPlaneInfo, VkDisplayPlaneCapabilities2KHR *pCapabilities)
    {
        return vkGetDisplayPlaneCapabilities2KHR(handle, pDisplayPlaneInfo, pCapabilities);
    }

    VkResult GetCalibrateableTimeDomainsEXT(uint32_t *pTimeDomainCount, VkTimeDomainEXT *pTimeDomains)
    {
        return vkGetPhysicalDeviceCalibrateableTimeDomainsEXT(handle, pTimeDomainCount, pTimeDomains);
    }

    VkResult GetCooperativeMatrixPropertiesNV(uint32_t *pPropertyCount, VkCooperativeMatrixPropertiesNV *pProperties)
    {
        return vkGetPhysicalDeviceCooperativeMatrixPropertiesNV(handle, pPropertyCount, pProperties);
    }

#ifdef _WIN32
    VkResult GetDrmDisplayEXT(int32_t drmFd, uint32_t connectorId, VkDisplayKHR *display)
    {
        return vkGetDrmDisplayEXT(handle, drmFd, connectorId, display);
    }

    VkResult GetOpticalFlowImageFormatsNV(VkOpticalFlowImageFormatInfoNV const *pOpticalFlowImageFormatInfo, uint32_t *pFormatCount, VkOpticalFlowImageFormatPropertiesNV *pImageFormatProperties)
    {
        return vkGetPhysicalDeviceOpticalFlowImageFormatsNV(handle, pOpticalFlowImageFormatInfo, pFormatCount, pImageFormatProperties);
    }

    VkResult AcquireDrmDisplayEXT(int32_t drmFd, VkDisplayKHR display)
    {
        return vkAcquireDrmDisplayEXT(handle, drmFd, display);
    }

    VkResult GetVideoFormatPropertiesKHR(VkPhysicalDeviceVideoFormatInfoKHR const *pVideoFormatInfo, uint32_t *pVideoFormatPropertyCount, VkVideoFormatPropertiesKHR *pVideoFormatProperties)
    {
        return vkGetPhysicalDeviceVideoFormatPropertiesKHR(handle, pVideoFormatInfo, pVideoFormatPropertyCount, pVideoFormatProperties);
    }

    VkResult GetVideoCapabilitiesKHR(VkVideoProfileInfoKHR const *pVideoProfile, VkVideoCapabilitiesKHR *pCapabilities)
    {
        return vkGetPhysicalDeviceVideoCapabilitiesKHR(handle, pVideoProfile, pCapabilities);
    }

    VkResult GetFragmentShadingRatesKHR(uint32_t *pFragmentShadingRateCount, VkPhysicalDeviceFragmentShadingRateKHR *pFragmentShadingRates)
    {
        return vkGetPhysicalDeviceFragmentShadingRatesKHR(handle, pFragmentShadingRateCount, pFragmentShadingRates);
    }

    VkResult GetToolProperties(uint32_t *pToolCount, VkPhysicalDeviceToolProperties *pToolProperties)
    {
        return vkGetPhysicalDeviceToolProperties(handle, pToolCount, pToolProperties);
    }

    VkResult GetSupportedFramebufferMixedSamplesCombinationsNV(uint32_t *pCombinationCount, VkFramebufferMixedSamplesCombinationNV *pCombinations)
    {
        return vkGetPhysicalDeviceSupportedFramebufferMixedSamplesCombinationsNV(handle, pCombinationCount, pCombinations);
    }

    void GetFamilyPerformanceQueryPassesKHR(VkQueryPoolPerformanceCreateInfoKHR const *pPerformanceQueryCreateInfo, uint32_t *pNumPasses)
    {
        vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR(handle, pPerformanceQueryCreateInfo, pNumPasses);
    }

    VkResult EnumerateFamilyPerformanceQueryCountersKHR(uint32_t queueFamilyIndex, uint32_t *pCounterCount, VkPerformanceCounterKHR *pCounters, VkPerformanceCounterDescriptionKHR *pCounterDescriptions)
    {
        return vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR(handle, queueFamilyIndex, pCounterCount, pCounters, pCounterDescriptions);
    }

    VkResult GetSurfacePresentModes2EXT(VkPhysicalDeviceSurfaceInfo2KHR const *pSurfaceInfo, uint32_t *pPresentModeCount, VkPresentModeKHR *pPresentModes)
    {
        return vkGetPhysicalDeviceSurfacePresentModes2EXT(handle, pSurfaceInfo, pPresentModeCount, pPresentModes);
    }

    VkBool32 GetWin32PresentationSupportKHR(uint32_t queueFamilyIndex)
    {
        return vkGetPhysicalDeviceWin32PresentationSupportKHR(handle, queueFamilyIndex);
    }
#endif

public:
    PhysicalDevice() = default;

    PhysicalDevice(Instance *instance, VkPhysicalDevice physicalDevice);

    PhysicalDevice(PhysicalDevice &&other);

    PhysicalDevice &operator =(PhysicalDevice &&other);

    void Swap(PhysicalDevice &other);

    PhysicalDevice(const PhysicalDevice &other) = delete;

    PhysicalDevice &operator =(const PhysicalDevice &other) = delete;

    void Activate(Feature feature) const;

    void Deactivate(Feature feature) const;

    template <class T>
    VkResult EnumerateDeviceExtensionProperties(T &container)
    {
        uint32_t propsCount;
        VkResult ret = EnumerateDeviceExtensionProperties(nullptr, &propsCount, nullptr);
        if (ret != VK_SUCCESS)
        {
            return ret;
        }

        container.resize(propsCount);
        return EnumerateDeviceExtensionProperties(nullptr, &propsCount, container.data());
    }

    template <class T>
    T Get()
    {
        if constexpr (IsPrimitiveOf<Instance, T>())
        {
            return *instance;
        }
    }

    VkFormat GetSuitableDepthFormat(bool depthOnly = false)
    {
        return SuitableDepthFormat(handle, depthOnly);
    }

    bool IsPresentSupported(VkSurfaceKHR surface, UINT32 queueFamilyIndex)
    {
        VkBool32 presentSupported = VK_FALSE;
        Check(GetSurfaceSupportKHR(queueFamilyIndex, surface, &presentSupported));

        return !!presentSupported;
    }

    template <typename T>
    T *RequestExtensionFeatures(VkStructureType type)
    {
        THROWIF(!instance->IsEnabled(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME),
            "Couldn't request feature from device as " VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME " isn't enabled!");

        const auto &it = ExtensionFeatures.find(type);
        if (it != ExtensionFeatures.end())
        {
            return (T *)it->second.get();
        }

        auto shared = std::make_shared<T>(T{ type });
        T *pExtension = shared.get();
        VkPhysicalDeviceFeatures2KHR physicalDeviceFeatures{};
        physicalDeviceFeatures.sType    = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2_KHR;
        physicalDeviceFeatures.pNext    = pExtension;
        physicalDeviceFeatures.features = Features;
        GetFeatures2(&physicalDeviceFeatures);

        ExtensionFeatures.insert({ type, shared });
        if (LastRequestedExtensionFeature)
        {
            pExtension->pNext = LastRequestedExtensionFeature;
        }
        LastRequestedExtensionFeature = pExtension;

        return pExtension;
    }

    void Invalidate(Instance *_instance)
    {
        instance = _instance;
    }

protected:
    Instance *instance;

public:
    std::vector<VkQueueFamilyProperties> QueueFamilyProperties;

    std::unordered_map<VkStructureType, std::shared_ptr<void>> ExtensionFeatures;

    VkPhysicalDeviceFeatures Features;

    VkPhysicalDeviceProperties Properties;

    VkPhysicalDeviceMemoryProperties MemoryProperties;

    VkPhysicalDeviceFeatures RequestedFeatures;

    Anonymous LastRequestedExtensionFeature{ nullptr };

    bool HighPriorityGraphicsQueue = true;
};

}
}
