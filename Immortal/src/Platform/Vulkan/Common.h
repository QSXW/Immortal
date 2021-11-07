#pragma once

#ifndef __VULKAN_COMMON_H__
#define __VULKAN_COMMON_H__
#define GLFW_INCLUDE_VULKAN

#include "volk.h"
#include "vk_mem_alloc.h"
#include "ImmortalCore.h"

namespace Immortal
{
namespace Vulkan
{
static struct VolkDeviceTable DeviceMap {};

struct Semaphores
{
    VkSemaphore acquiredImageReady;
    VkSemaphore renderComplete;
};

class CommandBuffer;
using CommandBuffers = std::vector<CommandBuffer *>;

using ThreadIndex = UINT32;
using FrameIndex  = UINT32;
using Surface     = VkSurfaceKHR;

enum class Level : int
{
    None      = -1,
    Primary   = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    Secondary = VK_COMMAND_BUFFER_LEVEL_SECONDARY,
    Max       = VK_COMMAND_BUFFER_LEVEL_MAX_ENUM
};

template <class T>
inline constexpr bool operator==(T a, int b)
{
    if constexpr (typeof<T, Level>())
    {
        return ncast<int>(a) == b;
    }
    else
    {
        a == b;
    }
}

template <class T>
inline constexpr bool operator==(int a, T b)
{
    if constexpr (typeof<T, Level>())
    {
        return ncast<int>(b) == a;
    }
    else
    {
        a == b;
    }
}

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

VkFormat SuitableDepthFormat(VkPhysicalDevice             physicalDevice,
                             bool                         depthOnly             = false,
                             const std::vector<VkFormat> &depthFormatPriorities = { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D16_UNORM });

static inline bool IsDepthOnlyFormat(VkFormat format)
{
    return format == VK_FORMAT_D16_UNORM || format == VK_FORMAT_D32_SFLOAT;
}
}
}

#endif // ! __VULKAN_COMMON_H__
