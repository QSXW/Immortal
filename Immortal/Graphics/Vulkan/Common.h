#pragma once

#ifndef VULKAN_COMMON_H__
#define VULKAN_COMMON_H__

#define GLFW_INCLUDE_VULKAN

#include "Core.h"
#include "volk.h"
#include "vk_mem_alloc.h"
#include "Interface/IObject.h"

namespace Immortal
{
namespace Vulkan
{

#define VKCPP_SWAPPABLE(T) T(T &&other) : T{} { other.Swap(*this); } T &operator=(T &&other) { T(std::move(other)).Swap(*this); return *this; }  T(const T &) = delete; T &operator=(const T &) = delete;
#define VKCPP_OPERATOR_HANDLE() Primitive Handle() const { return handle; } operator Primitive() const { return handle; } protected: Primitive handle{ VK_NULL_HANDLE }; public: operator bool() const { return handle != VK_NULL_HANDLE; }

namespace Limit
{

constexpr int PushConstantMaxSize = 128;

static std::vector<VkDescriptorPoolSize> PoolSize{
    { VK_DESCRIPTOR_TYPE_SAMPLER,                1000 },
    { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
    { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,          1000 },
    { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,          1000 },
    { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,   1000 },
    { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,   1000 },
    { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         1000 },
    { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,         1000 },
    { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
    { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
    { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,       1000 }
};

}

/**
 * @brief Vulkan API
 */
static inline VkResult EnumerateInstanceVersion(uint32_t *pApiVersion)
{
    return vkEnumerateInstanceVersion(pApiVersion);
}

static inline VkResult EnumerateInstanceLayerProperties(uint32_t *pPropertyCount, VkLayerProperties *pProperties)
{
    return vkEnumerateInstanceLayerProperties(pPropertyCount, pProperties);
}

static inline VkResult EnumerateInstanceExtensionProperties(char const *pLayerName, uint32_t *pPropertyCount, VkExtensionProperties *pProperties)
{
    return vkEnumerateInstanceExtensionProperties(pLayerName, pPropertyCount, pProperties);
}

static inline VkResult CreateInstance(VkInstanceCreateInfo const *pCreateInfo, VkInstance *pInstance, VkAllocationCallbacks const *pAllocator = nullptr)
{
    return vkCreateInstance(pCreateInfo, pAllocator, pInstance);
}

#define VK_CASE(x) case x: return #x;

inline const char *Stringify(VkResult err)
{
    switch (err)
    {
        VK_CASE(VK_SUCCESS)
        VK_CASE(VK_NOT_READY)
        VK_CASE(VK_TIMEOUT)
        VK_CASE(VK_EVENT_SET)
        VK_CASE(VK_EVENT_RESET)
        VK_CASE(VK_INCOMPLETE)
        VK_CASE(VK_ERROR_OUT_OF_HOST_MEMORY)
        VK_CASE(VK_ERROR_OUT_OF_DEVICE_MEMORY)
        VK_CASE(VK_ERROR_INITIALIZATION_FAILED)
        VK_CASE(VK_ERROR_DEVICE_LOST)
        VK_CASE(VK_ERROR_MEMORY_MAP_FAILED)
        VK_CASE(VK_ERROR_LAYER_NOT_PRESENT)
        VK_CASE(VK_ERROR_EXTENSION_NOT_PRESENT)
        VK_CASE(VK_ERROR_FEATURE_NOT_PRESENT)
        VK_CASE(VK_ERROR_INCOMPATIBLE_DRIVER)
        VK_CASE(VK_ERROR_TOO_MANY_OBJECTS)
        VK_CASE(VK_ERROR_FORMAT_NOT_SUPPORTED)
        VK_CASE(VK_ERROR_FRAGMENTED_POOL)
        VK_CASE(VK_ERROR_UNKNOWN)
        VK_CASE(VK_ERROR_OUT_OF_POOL_MEMORY)
        VK_CASE(VK_ERROR_INVALID_EXTERNAL_HANDLE)
        VK_CASE(VK_ERROR_FRAGMENTATION)
        VK_CASE(VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS)
        VK_CASE(VK_ERROR_SURFACE_LOST_KHR)
        VK_CASE(VK_ERROR_NATIVE_WINDOW_IN_USE_KHR)
        VK_CASE(VK_SUBOPTIMAL_KHR)
        VK_CASE(VK_ERROR_OUT_OF_DATE_KHR)
        VK_CASE(VK_ERROR_INCOMPATIBLE_DISPLAY_KHR)
        VK_CASE(VK_ERROR_VALIDATION_FAILED_EXT)
        VK_CASE(VK_ERROR_INVALID_SHADER_NV)
        VK_CASE(VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT)
        VK_CASE(VK_ERROR_NOT_PERMITTED_EXT)
        VK_CASE(VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT)
        VK_CASE(VK_THREAD_IDLE_KHR)
        VK_CASE(VK_THREAD_DONE_KHR)
        VK_CASE(VK_OPERATION_DEFERRED_KHR)
        VK_CASE(VK_OPERATION_NOT_DEFERRED_KHR)
        VK_CASE(VK_PIPELINE_COMPILE_REQUIRED_EXT)
        VK_CASE(VK_RESULT_MAX_ENUM)
        VK_CASE(VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR)
        VK_CASE(VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR)
        VK_CASE(VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR)
        VK_CASE(VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR)
        VK_CASE(VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR)
        VK_CASE(VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR)
        VK_CASE(VK_ERROR_COMPRESSION_EXHAUSTED_EXT)
        default: return "VK_ERROR_UNKNOWN";
    }
}

const char *Stringify(VkFormat format);
const char *Stringify(VkPresentModeKHR presentMode);
const char *Stringify(VkImageUsageFlagBits imageUsage);
const char *Stringify(VkSurfaceTransformFlagBitsKHR transform);
const char *Stringify(VkCompositeAlphaFlagBitsKHR compositeAlpha);

inline const std::string Stringify(VkSurfaceFormatKHR surfaceFormat)
{
    std::string surfaceFormatString = std::string(Stringify(surfaceFormat.format)) + ", ";

    switch (surfaceFormat.colorSpace)
    {
    case VK_COLORSPACE_SRGB_NONLINEAR_KHR:
        surfaceFormatString += "VK_COLORSPACE_SRGB_NONLINEAR_KHR";
        break;
    default:
        surfaceFormatString += "UNKNOWN COLOR SPACE";
    }
    return surfaceFormatString;
}

#if defined( _DEBUG )
inline void Check(VkResult status)
{
    if (status)
    {
        LOG::ERR("Detected Vulkan error: {}", Stringify(status));
        abort();
    }
}
#else
constexpr inline void Check(VkResult err) { }
#endif

VkFormat SuitableDepthFormat(VkPhysicalDevice             physicalDevice,
                             bool                         depthOnly             = false,
                             const std::vector<VkFormat> &depthFormatPriorities = { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D16_UNORM });

static inline bool IsDepthOnlyFormat(VkFormat format)
{
    return format == VK_FORMAT_D16_UNORM || format == VK_FORMAT_D32_SFLOAT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

}
}

#endif // ! __VULKAN_COMMON_H__
