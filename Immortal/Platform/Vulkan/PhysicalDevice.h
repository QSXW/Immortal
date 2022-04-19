#pragma once

#include "Core.h"
#include "Common.h"

#include "Instance.h"

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
    PhysicalDevice() = default;

    PhysicalDevice(Instance *instance, VkPhysicalDevice physicalDevice);

    void Activate(Feature feature) const;

    void Deactivate(Feature feature) const;

public: /* vk api */
    VkResult GetCapabilities(VkSurfaceKHR surface, VkSurfaceCapabilitiesKHR *pSurfaceCapabilities) const
    {
        return vkGetPhysicalDeviceSurfaceCapabilitiesKHR(handle, surface, pSurfaceCapabilities);
    }

    VkResult EnumerateDeviceExtensionProperties(const char *pLayerName, uint32_t *pPropertyCount, VkExtensionProperties* pProperties) const
    {
        return vkEnumerateDeviceExtensionProperties(handle, pLayerName, pPropertyCount, pProperties);
    }

    VkResult CreateDevice(const VkDeviceCreateInfo *pCreateInfo, VkDevice *pDevice, const VkAllocationCallbacks *pAllocator = nullptr) const
    {
        return vkCreateDevice(handle, pCreateInfo, pAllocator, pDevice);
    }
    
    VkResult GetSurfaceSupport(uint32_t queueFamilyIndex, VkSurfaceKHR surface, VkBool32 *pSupported) const
    {
        return vkGetPhysicalDeviceSurfaceSupportKHR(handle, queueFamilyIndex, surface, pSupported);
    }

    void GetQueueFamilyProperties(uint32_t *pQueueFamilyPropertyCount, VkQueueFamilyProperties *pQueueFamilyProperties) const
    {
        vkGetPhysicalDeviceQueueFamilyProperties(handle, pQueueFamilyPropertyCount, pQueueFamilyProperties);
    }

#define DEFINE_PHYSICAL_DEVICE_GETTER(T) \
    void Get##T(VkPhysicalDevice##T *pType) const \
    { \
        return vkGetPhysicalDevice##T(handle, pType); \
    }

    DEFINE_PHYSICAL_DEVICE_GETTER(Features)
    DEFINE_PHYSICAL_DEVICE_GETTER(Features2KHR)
    DEFINE_PHYSICAL_DEVICE_GETTER(Properties)
    DEFINE_PHYSICAL_DEVICE_GETTER(MemoryProperties)

public:
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
        Check(GetSurfaceSupport(queueFamilyIndex, surface, &presentSupported));

        return !!presentSupported;
    }

    template <typename T>
    T &RequestExtensionFeatures(VkStructureType type)
    {
        if (!instance->IsEnabled(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME))
        {
            LOG::ERR(
                "Couldn't request feature from device as "
                VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME
                " isn't enabled!"
            );
        }

        const auto &it = ExtensionFeatures.find(type);
        if (it != ExtensionFeatures.end())
        {
            return *static_cast<T *>(it->second.get());
        }

        T extension{ type };
        VkPhysicalDeviceFeatures2KHR physicalDeviceFeatures{};
        physicalDeviceFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2_KHR;
        physicalDeviceFeatures.pNext = &extension;
        physicalDeviceFeatures.features = Features;

        GetFeatures2KHR(&physicalDeviceFeatures);

        auto pExtension = std::make_shared<T>(extension);
        ExtensionFeatures.insert({ type, pExtension });

        if (LastRequestedExtensionFeature)
        {
            pExtension->pNext = LastRequestedExtensionFeature;
        }
        LastRequestedExtensionFeature = pExtension.get();

        return *pExtension;
    }

private:
    Instance *instance{ nullptr };

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
