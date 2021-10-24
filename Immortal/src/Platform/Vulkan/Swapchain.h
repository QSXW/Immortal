#pragma once

#include "ImmortalCore.h"

#include "Common.h"
#include "Device.h"

namespace Immortal
{
namespace Vulkan
{

class Swapchain
{
public:
    struct Properties
    {
        VkSwapchainKHR                OldSwapchain;
        UINT32                        ImageCount{ 3 };
        VkExtent2D                    Extent{};
        VkSurfaceFormatKHR            SurfaceFormat{};
        UINT32                        ArrayLayers{ 1 };
        VkImageUsageFlags		      ImageUsage;
        VkSurfaceTransformFlagBitsKHR PreTransform;
        VkCompositeAlphaFlagBitsKHR   CompositeAlpha;
        VkPresentModeKHR              PresentMode;
    };

public:
    Swapchain(Swapchain &swapchain, const VkExtent2D &extent, const VkSurfaceTransformFlagBitsKHR transform);

    Swapchain(Device                              &device,
              VkSurfaceKHR                         surface,
              const VkExtent2D                    &extent = {},
              const UINT32                         imageCount       = 3,
              const VkSurfaceTransformFlagBitsKHR  transform        = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
              const VkPresentModeKHR               presentMode      = VK_PRESENT_MODE_FIFO_KHR,
              const std::set<VkImageUsageFlagBits> &imageUsageFlags = { VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_USAGE_TRANSFER_SRC_BIT });

    Swapchain(Swapchain                           &oldSwapchain,
              Device                              &device,
              VkSurfaceKHR                         surface,
              const VkExtent2D                    &extent = {},
              const uint32_t                       imageCount       = 3,
              const VkSurfaceTransformFlagBitsKHR  transform        = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
              const VkPresentModeKHR               presentMode      = VK_PRESENT_MODE_FIFO_KHR,
              const std::set<VkImageUsageFlagBits> &imageUsageFlags = { VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_USAGE_TRANSFER_SRC_BIT });

    ~Swapchain();

    void Create();

public:
    using Images = std::vector<VkImage>;
    using Format = VkFormat;
    using Usage  = VkImageUsageFlags;

    VkSwapchainKHR &Handle()
    {
        return handle;
    }

    template <class T>
    T &Get()
    {
        if constexpr (is_same<T, VkImageUsageFlags>())
        {
            return properties.ImageUsage;
        }
        if constexpr (is_same<T, Properties>())
        {
            return properties;
        }
        if constexpr (is_same<T, VkExtent2D>())
        {
            return properties.Extent;
        }
        if constexpr (is_same <T, VkFormat>())
        {
            return properties.SurfaceFormat.format;
        }
        if constexpr (is_same<T, Images>())
        {
            return images;
        }
        if constexpr (is_same<T, FrameIndex>())
        {
            return frameIndex;
        }
        if constexpr (is_same<T, Surface>())
        {
            return surface;
        }
    }

    void Set(const VkPresentModeKHR mode)
    {
        properties.PresentMode = mode;
    }

    void Set(const VkFormat format)
    {
        properties.SurfaceFormat.format = format;
    }

    void Set(std::vector<VkPresentModeKHR> &presentModePriorities)
    {
        presentModePriorities = presentModePriorities;
    }

    void Set(std::vector<VkSurfaceFormatKHR> &surfaceFormatPriorities)
    {
        surfaceFormatPriorities = surfaceFormatPriorities;
    }

    VkResult AcquireNextImage(UINT32 *index, VkSemaphore semaphore, VkFence fence)
    {
        return vkAcquireNextImageKHR(device.Handle(), handle, std::numeric_limits<uint64_t>::max(), semaphore, fence, index);
    }

private:
    Device &device;

    VkSurfaceKHR surface{ VK_NULL_HANDLE };

    VkSwapchainKHR handle{ VK_NULL_HANDLE };

    FrameIndex frameIndex{ 0 };

    std::vector<VkImage> images;

    std::vector<VkSurfaceFormatKHR> surfaceFormats{};

    std::vector<VkPresentModeKHR> presentModes{};

    Swapchain::Properties properties;

    std::vector<VkPresentModeKHR> presentModePriorities = {
        VK_PRESENT_MODE_FIFO_KHR,
        VK_PRESENT_MODE_MAILBOX_KHR
    };

    std::vector<VkSurfaceFormatKHR> surfaceFormatPriorities = {
        VkSurfaceFormatKHR{ VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR },
        VkSurfaceFormatKHR{ VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR },
        VkSurfaceFormatKHR{ VK_FORMAT_R8G8B8A8_SRGB,  VK_COLOR_SPACE_SRGB_NONLINEAR_KHR },
        VkSurfaceFormatKHR{ VK_FORMAT_B8G8R8A8_SRGB,  VK_COLOR_SPACE_SRGB_NONLINEAR_KHR },
    };

    VkPresentModeKHR presentMode{};

    std::set<VkImageUsageFlagBits> imageUsageFlags;
};

}
}
