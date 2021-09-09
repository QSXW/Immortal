#pragma once

#include "volk.h"
#include "vk_mem_alloc.h"
#include "ImmortalCore.h"

namespace Immortal
{
namespace Vulkan
{
#define VK_LAYER_LUNARG_API_DUMP              "VK_LAYER_LUNARG_api_dump"
#define VK_LAYER_LUNARG_DEVICE_SIMULATION     "VK_LAYER_LUNARG_device_simulation"
#define VK_LAYER_LAYER_LUNARG_ASSISTANT_LAYER "VK_LAYER_LUNARG_assistant_layer"
#define VK_LAYER_KHRONOS_VALIDATION           "VK_LAYER_KHRONOS_validation"
#define VK_LAYER_LUNARG_MONITOR               "VK_LAYER_LUNARG_monitor"
#define VK_LAYER_LUNARG_SCREENSHOT            "VK_LAYER_LUNARG_screenshot"

using Extent2D = VkExtent2D;
using Extent3D = VkExtent3D;

template <class T>
constexpr inline UINT32 U32(T x)
{
    return static_cast<UINT32>(x);
}

template <class T, class F>
constexpr inline void IfNotNullThen(F func, T handle, const VkAllocationCallbacks* pAllocator = nullptr)
{
    if (handle != VK_NULL_HANDLE)
    {
        func(handle, pAllocator);
    }
}

template <class T, class F>
constexpr inline void IfNotNullThen(F func, VkDevice device, T handle, const VkAllocationCallbacks* pAllocator = nullptr)
{
    if (handle != VK_NULL_HANDLE)
    {
        func(device, handle, pAllocator);
    }
}

template <class T, void (*F)(VkDevice, T, const VkAllocationCallbacks *)>
constexpr inline void IfNotNullThen(VkDevice device, T handle)
{
    if (handle != VK_NULL_HANDLE)
    {
        F(device, handle, nullptr);
    }
}

template <class T, void (*F)(T, const VkAllocationCallbacks *)>
constexpr inline void IfNotNullThen(T handle)
{
    if (handle != VK_NULL_HANDLE)
    {
        F(handle, nullptr);
    }
}

template <class T, void (*F)(T)>
constexpr inline void IfNotNullThen(T handle)
{
    if (handle != VK_NULL_HANDLE)
    {
        F(handle);
    }
}

inline const char *Stringify(VkResult err)
{
#define XX(x) case x: return #x;
    switch (err)
    {
        XX(VK_SUCCESS)
        XX(VK_NOT_READY)
        XX(VK_TIMEOUT)
        XX(VK_EVENT_SET)
        XX(VK_EVENT_RESET)
        XX(VK_INCOMPLETE)
        XX(VK_ERROR_OUT_OF_HOST_MEMORY)
        XX(VK_ERROR_OUT_OF_DEVICE_MEMORY)
        XX(VK_ERROR_INITIALIZATION_FAILED)
        XX(VK_ERROR_DEVICE_LOST)
        XX(VK_ERROR_MEMORY_MAP_FAILED)
        XX(VK_ERROR_LAYER_NOT_PRESENT)
        XX(VK_ERROR_EXTENSION_NOT_PRESENT)
        XX(VK_ERROR_FEATURE_NOT_PRESENT)
        XX(VK_ERROR_INCOMPATIBLE_DRIVER)
        XX(VK_ERROR_TOO_MANY_OBJECTS)
        XX(VK_ERROR_FORMAT_NOT_SUPPORTED)
        XX(VK_ERROR_FRAGMENTED_POOL)
        XX(VK_ERROR_UNKNOWN)
        XX(VK_ERROR_OUT_OF_POOL_MEMORY)
        XX(VK_ERROR_INVALID_EXTERNAL_HANDLE)
        XX(VK_ERROR_FRAGMENTATION)
        XX(VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS)
        XX(VK_ERROR_SURFACE_LOST_KHR)
        XX(VK_ERROR_NATIVE_WINDOW_IN_USE_KHR)
        XX(VK_SUBOPTIMAL_KHR)
        XX(VK_ERROR_OUT_OF_DATE_KHR)
        XX(VK_ERROR_INCOMPATIBLE_DISPLAY_KHR)
        XX(VK_ERROR_VALIDATION_FAILED_EXT)
        XX(VK_ERROR_INVALID_SHADER_NV)
        XX(VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT)
        XX(VK_ERROR_NOT_PERMITTED_EXT)
        XX(VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT)
        XX(VK_THREAD_IDLE_KHR)
        XX(VK_THREAD_DONE_KHR)
        XX(VK_OPERATION_DEFERRED_KHR)
        XX(VK_OPERATION_NOT_DEFERRED_KHR)
        XX(VK_PIPELINE_COMPILE_REQUIRED_EXT)
    }
    return "";
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
        
#if defined( DEBUG ) || defined( _DEBUG )
inline void Check(VkResult status)
{
    auto msg = Stringify(status);
    if (status)
    {
        LOG::ERR("Detected Vulkan error: {0}", msg);
        abort();
    }
}
#else
constexpr inline void Check(VkResult err) { }
#endif

static inline bool Equals(const char *str1, const char *str2)
{
    return (strcmp(str1, str2) == 0);
}

VkFormat SuitableDepthFormat(VkPhysicalDevice             physicalDevice,
                                bool                         depthOnly             = false,
                                const std::vector<VkFormat> &depthFormatPriorities = { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D16_UNORM });

static inline bool IsDepthOnlyFormat(VkFormat format)
{
    return format == VK_FORMAT_D16_UNORM || format == VK_FORMAT_D32_SFLOAT;
}
}
}
