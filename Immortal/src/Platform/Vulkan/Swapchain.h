#pragma once

#include "ImmortalCore.h"

#include "vkcommon.h"
#include "Device.h"

namespace Immortal
{
    namespace Vulkan
    {
        /**
         * @brief The swap chain is essentially a queue of images that are waiting to be presented to the screen.
         */
    class Swapchain
    {
    public:

        /**
            * @ArrayLayers specifies the amount of layers each image consists of. This is always 1 unless you are developing a stereoscopic 3D application.
            * @ImageUsage  specifies what kind of operations we'll use the images in the swap chain for.
            */
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
        Swapchain(Device                              &device,
                  VkSurfaceKHR                         surface,
                  const VkExtent2D                    &extent = {},
                  const UINT32                         imageCount       = 3,
                  const VkSurfaceTransformFlagBitsKHR  transform        = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
                  const VkPresentModeKHR               presentMode      = VK_PRESENT_MODE_FIFO_KHR,
                  const std::set<VkImageUsageFlagBits> &imageUsageFlags = { VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_USAGE_TRANSFER_SRC_BIT });

        Swapchain(Swapchain                           &oldSwapchain,
                  Device                               &device,
                  VkSurfaceKHR                         surface,
                  const VkExtent2D                    &extent = {},
                  const uint32_t                       imageCount       = 3,
                  const VkSurfaceTransformFlagBitsKHR  transform        = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
                  const VkPresentModeKHR               presentMode      = VK_PRESENT_MODE_FIFO_KHR,
                  const std::set<VkImageUsageFlagBits> &imageUsageFlags = { VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_USAGE_TRANSFER_SRC_BIT });

        ~Swapchain();

        void Create();

    public:
        template <class T>
        T &Get()
        {
            if constexpr (std::is_same_v<T, Properties>)
            {
                return properties;
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

        std::vector<VkImage> &Images()
        {
            return images;
        }

        VkFormat &Format()
        {
            return properties.SurfaceFormat.format;
        }

        VkImageUsageFlags &Usage()
        {
            return properties.ImageUsage;
        }

    public:
        DefineGetHandleFunc(VkSwapchainKHR)

    private:
        Device &device;
        VkSurfaceKHR surface{ VK_NULL_HANDLE };
        VkSwapchainKHR handle{ VK_NULL_HANDLE };

        std::vector<VkImage> images;
        std::vector<VkSurfaceFormatKHR> surfaceFormats{};
        std::vector<VkPresentModeKHR> presentModes{};

        Swapchain::Properties properties;

        // A list of present modes in order of priority (index is the priority 0 - hight, And so on.)
        std::vector<VkPresentModeKHR> presentModePriorities = {
            VK_PRESENT_MODE_FIFO_KHR,
            VK_PRESENT_MODE_MAILBOX_KHR
        };

        // A list of surface formats in order of priority (index is the priority 0 - hight, And so on.)
        std::vector<VkSurfaceFormatKHR> surfaceFormatPriorities = {
            VkSurfaceFormatKHR{ VK_FORMAT_R8G8B8A8_SRGB,  VK_COLOR_SPACE_SRGB_NONLINEAR_KHR },
            VkSurfaceFormatKHR{ VK_FORMAT_B8G8R8A8_SRGB,  VK_COLOR_SPACE_SRGB_NONLINEAR_KHR },
            VkSurfaceFormatKHR{ VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR },
            VkSurfaceFormatKHR{ VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR }
        };

        VkPresentModeKHR presentMode{};

        std::set<VkImageUsageFlagBits> mImageUsageFlags;
    };
}
}
