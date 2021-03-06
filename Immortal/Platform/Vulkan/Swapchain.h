#pragma once

#include "Core.h"

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
        uint32_t                      ImageCount{ 3 };
        VkExtent2D                    Extent{};
        VkSurfaceFormatKHR            SurfaceFormat{};
        uint32_t                      ArrayLayers{ 1 };
        VkImageUsageFlags		      ImageUsage;
        VkSurfaceTransformFlagBitsKHR PreTransform;
        VkCompositeAlphaFlagBitsKHR   CompositeAlpha;
        VkPresentModeKHR              PresentMode;
    };

    static constexpr uint32_t MaxFrameCount = 3;

    static inline struct Priorities
    {
        Priorities() :
            PresentMode{
                { VK_PRESENT_MODE_FIFO_KHR      },
                { VK_PRESENT_MODE_MAILBOX_KHR   },
                { VK_PRESENT_MODE_IMMEDIATE_KHR },
            },
            SurfaceFormat{
                VkSurfaceFormatKHR{ VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR },
                VkSurfaceFormatKHR{ VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR },
                VkSurfaceFormatKHR{ VK_FORMAT_R8G8B8A8_SRGB,  VK_COLOR_SPACE_SRGB_NONLINEAR_KHR },
                VkSurfaceFormatKHR{ VK_FORMAT_B8G8R8A8_SRGB,  VK_COLOR_SPACE_SRGB_NONLINEAR_KHR },
            }
        {

        }
        std::vector<VkPresentModeKHR> PresentMode;

        std::vector<VkSurfaceFormatKHR> SurfaceFormat;
    } priorities;

    using Primitive = VkSwapchainKHR;
    VKCPP_OPERATOR_HANDLE()

public:
    Swapchain(Swapchain &swapchain, const VkExtent2D &extent, const VkSurfaceTransformFlagBitsKHR transform);

    Swapchain(Device                              *device,
              const VkExtent2D                    &extent      = {},
              const uint32_t                       imageCount  = 3,
              const VkSurfaceTransformFlagBitsKHR  transform   = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
              const VkPresentModeKHR               presentMode = VK_PRESENT_MODE_FIFO_KHR,
              VkImageUsageFlags                    imageUsage  = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);

    Swapchain(Swapchain                           &oldSwapchain,
              Device                              *device,
              const VkExtent2D                    &extent      = {},
              const uint32_t                       imageCount  = 3,
              const VkSurfaceTransformFlagBitsKHR  transform   = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
              const VkPresentModeKHR               presentMode = VK_PRESENT_MODE_FIFO_KHR,
              VkImageUsageFlags                    imageUsage  = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);

    ~Swapchain();

    void Create();

public:
    using Images = std::vector<VkImage>;
    using Format = VkFormat;
    using Usage  = VkImageUsageFlags;

    template <class T>
    T &Get()
    {
        if constexpr (IsPrimitiveOf<VkImageUsageFlags, T>())
        {
            return properties.ImageUsage;
        }
        if constexpr (IsPrimitiveOf<Properties, T>())
        {
            return properties;
        }
        if constexpr (IsPrimitiveOf<VkExtent2D, T>())
        {
            return properties.Extent;
        }
        if constexpr (IsPrimitiveOf <VkFormat, T>())
        {
            return properties.SurfaceFormat.format;
        }
        if constexpr (IsPrimitiveOf<Images, T>())
        {
            return images;
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

    VkResult AcquireNextImage(uint32_t *index, VkSemaphore semaphore, VkFence fence)
    {
        return vkAcquireNextImageKHR(*device, handle, std::numeric_limits<uint32_t>::max(), semaphore, fence, index);
    }

private:
    Device *device{ nullptr };

    std::vector<VkImage> images;

    std::vector<VkSurfaceFormatKHR> surfaceFormats{};

    std::vector<VkPresentModeKHR> presentModes{};

    Swapchain::Properties properties;
};

}
}
