#include "impch.h"
#include "Swapchain.h"

#include "Framework/Utils.h"

namespace Immortal
{
namespace Vulkan
{
    inline UINT32 SelectImageCount(UINT32 request, UINT32 min, UINT32 max)
    {
        Utils::Clamp(request, min, max);
        return request;
    }

    inline UINT32 SelectImageArrayLayers(UINT32 request, UINT32 max)
    {
        Utils::Clamp(request, 1u, max);
        return request;
    }

    inline VkExtent2D SelectExtent(VkExtent2D request, const VkExtent2D &min, const VkExtent2D &max, const VkExtent2D &current)
    {
        if (request.width < 1 || request.height < 1)
        {
            Log::Warn("(Swapchain) Image extent ({0}, {1}) not supported. Selecting ({2}, {3}).",
                request.width, request.height, current.width, current.height);
            return current;
        }

        Utils::Clamp<UINT32>(request.width, min.width, max.width);
        Utils::Clamp<UINT32>(request.height, min.height, max.height);
        return request;
    }

    inline VkSurfaceFormatKHR SelectSurfaceFormat(const VkSurfaceFormatKHR request, const std::vector<VkSurfaceFormatKHR> &available, const std::vector<VkSurfaceFormatKHR> &priorities)
    {
        auto it = std::find_if(available.begin(), available.end(), [&request](const VkSurfaceFormatKHR &surface)
            {
                if (surface.format == request.format && surface.colorSpace == request.colorSpace)
                {
                    return true;
                }
                return false;
            });

        // If the requested surface format isn't found, then try to request a format from the priority list
        if (it == available.end())
        {
            for (auto &f : priorities)
            {
                it = std::find_if(available.begin(), available.end(), [&f](const VkSurfaceFormatKHR &surface)
                    {
                        if (surface.format == f.format && surface.colorSpace == f.colorSpace)
                        {
                            return true;
                        }
                        return false;
                    });
                if (it != available.end())
                {
                __IM__000_000_001_ReturnSurfaceFormat:
                    Log::Warn("(Swapchain) Surface format ({0}) not supported. Selecting ({1}).",
                               Stringify(request), Stringify(*it));
                    return *it;
                }
            }
            it = available.begin();
            goto __IM__000_000_001_ReturnSurfaceFormat;
        }
        else
        {
            Log::Info("(Swapchain) Surface format selected: {0}", Stringify(request));
        }
        return *it;
    }

    inline bool ValidateFormatFeature(VkImageUsageFlagBits imageUsage, VkFormatFeatureFlags supportedFeatures)
    {
        switch (imageUsage)
        {
        case VK_IMAGE_USAGE_STORAGE_BIT:
            return VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT & supportedFeatures;
        default:
            return true;
        }
    }

    inline std::set<VkImageUsageFlagBits> SelectImageUsage(const std::set<VkImageUsageFlagBits> &request, VkImageUsageFlags support, VkFormatFeatureFlags supportedFeatures)
    {
        std::set<VkImageUsageFlagBits> validated;

        for (auto flag : request)
        {
            if ((flag & support) && ValidateFormatFeature(flag, supportedFeatures))
            {
                validated.insert(flag);
            }
            else
            {
                Log::Warn("(Swapchain) Image usage ({0}) requested but not supported.", Stringify(flag));
            }
        }

        if (validated.empty())
        {
            static const VkImageUsageFlagBits imageUsageFlags[] = {
                VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                VK_IMAGE_USAGE_STORAGE_BIT,
                VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_IMAGE_USAGE_TRANSFER_DST_BIT
            };

            for (size_t i = 0; i < ARRAY_LEN(imageUsageFlags); i++)
            {
                if ((imageUsageFlags[i] & support) && ValidateFormatFeature(imageUsageFlags[i], supportedFeatures))
                {
                    validated.insert(imageUsageFlags[i]);
                    break;
                }
            }
        }

        SLASSERT(!validated.empty() && "No compatible image usage found.");
#if     IMMORTAL_CHECK_DEBUG
        Log::Info("(Swapchain) Image usage flags:");
        for (auto &flag : validated)
        {
            Log::Info("  \t{0}", Stringify(flag));
        }
#endif

        return validated;
    }

    inline VkImageUsageFlags CompositeImageFlags(const std::set<VkImageUsageFlagBits> &imageUsageFlags)
    {
        VkImageUsageFlags imageUsage{ 0 };
        for (auto flag : imageUsageFlags)
        {
            imageUsage |= flag;
        }
        return imageUsage;
    }

    inline VkSurfaceTransformFlagBitsKHR SelectTransform(VkSurfaceTransformFlagBitsKHR request, VkSurfaceTransformFlagsKHR support, VkSurfaceTransformFlagBitsKHR current)
    {
        if (request & support)
        {
            return request;
        }

        Log::Warn("(Swapchain) Surface transform '{0}' not supported. Selecting '{1}'.", Stringify(request), Stringify(current));

        return current;
    }

    inline VkCompositeAlphaFlagBitsKHR SelectCompositeAlpha(VkCompositeAlphaFlagBitsKHR request, VkCompositeAlphaFlagsKHR support)
    {
        if (request & support)
        {
            return request;
        }

        static const VkCompositeAlphaFlagBitsKHR compositeAlphaFlags[] = {
            VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
            VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
            VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR
        };

        for (size_t i = 0; i < ARRAY_LEN(compositeAlphaFlags); i++)
        {
            if (compositeAlphaFlags[i] & support)
            {
                Log::Warn("(Swapchain) Composite alpha '{0}' not supported. Selecting '{1}.",
                          Stringify(request), Stringify(compositeAlphaFlags[i]));
                return compositeAlphaFlags[i];
            }
        }

        SLASSERT(false && "No compatible composite alpha found.");
        return Utils::NullValue<VkCompositeAlphaFlagBitsKHR>();
    }

    inline VkPresentModeKHR SelectPresentMode(VkPresentModeKHR request, const std::vector<VkPresentModeKHR> &available, const std::vector<VkPresentModeKHR> &priorities)
    {
        auto it = std::find(available.begin(), available.end(), request);

        if (it == available.end())
        {
            // If nothing found, always default to FIFO
            VkPresentModeKHR chosen = VK_PRESENT_MODE_FIFO_KHR;

            for (auto &presentMode : priorities)
            {
                if (std::find(available.begin(), available.end(), presentMode) != available.end())
                {
                    chosen = presentMode;
                }
            }

            Log::Warn("(Swapchain) Present mode '{}' not supported. Selecting '{}'."),
                      Stringify(request), Stringify(chosen);
            return chosen;
        }
        else
        {
            Log::Info("(Swapchain) Present mode selected: {0}"), Stringify(request);
            return *it;
        }
    }

    Swapchain::Swapchain(Device                               &device,
                         VkSurfaceKHR                         surface,
                         const VkExtent2D &extent,
                         const UINT32                         imageCount,
                         const VkSurfaceTransformFlagBitsKHR  transform,
                         const VkPresentModeKHR               presentMode,
                         const std::set<VkImageUsageFlagBits> &imageUsageFlags) :
        Swapchain{ *this, device, surface, extent, imageCount, transform, presentMode, imageUsageFlags }
    {

    }

    Swapchain::Swapchain(Swapchain &oldSwapchain,
                         Device                              &device,
                         VkSurfaceKHR                         surface,
                         const VkExtent2D                    &extent,
                         const uint32_t                       imageCount,
                         const VkSurfaceTransformFlagBitsKHR  transform,
                         const VkPresentModeKHR               presentMode,
                         const std::set<VkImageUsageFlagBits> &imageUsageFlags) :
        device{ device },
        surface{ surface }
    {
        presentModePriorities  = oldSwapchain.presentModePriorities;
        surfaceFormatPriorities = oldSwapchain.surfaceFormatPriorities;

        VkPhysicalDevice &physicalDeviceHandle = device.Get<PhysicalDevice>().Handle();
        VkSurfaceCapabilitiesKHR surfaceCapabilities{};
        Check(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDeviceHandle, surface, &surfaceCapabilities));

        UINT32 surfaceFormatCount{ 0U };
        Check(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDeviceHandle, surface, &surfaceFormatCount, nullptr));
        surfaceFormats.resize(surfaceFormatCount);
        Check(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDeviceHandle, surface, &surfaceFormatCount, surfaceFormats.data()));

#if     IMMORTAL_CHECK_DEBUG
        Log::Info("Surface supports the following surface formats:");
        for (auto &f : surfaceFormats)
        {
            Log::Info("  \t{0}", Vulkan::Stringify(f));
        }
#endif
        UINT32 presentModeCount{ 0U };
        Check(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDeviceHandle, surface, &presentModeCount, nullptr));
        presentModes.resize(presentModeCount);
        Check(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDeviceHandle, surface, &presentModeCount, presentModes.data()));

#if     IMMORTAL_CHECK_DEBUG
        Log::Info("Surface supports the following present modes:");
        for (auto &p : presentModes)
        {
             Log::Info("  \t{0}", Vulkan::Stringify(p));
        }
#endif

        this->properties.ImageCount    = SelectImageCount(imageCount, surfaceCapabilities.minImageCount, surfaceCapabilities.maxImageCount);
        this->properties.Extent        = SelectExtent(extent, surfaceCapabilities.minImageExtent, surfaceCapabilities.maxImageExtent, surfaceCapabilities.currentExtent);
        this->properties.ArrayLayers   = SelectImageArrayLayers(1U, surfaceCapabilities.maxImageArrayLayers);
        this->properties.SurfaceFormat = SelectSurfaceFormat(this->properties.SurfaceFormat, surfaceFormats, surfaceFormatPriorities);

        VkFormatProperties formatProperties;
        vkGetPhysicalDeviceFormatProperties(physicalDeviceHandle, this->properties.SurfaceFormat.format, &formatProperties);
        this->imageUsageFlags     = SelectImageUsage(imageUsageFlags, surfaceCapabilities.supportedUsageFlags, formatProperties.optimalTilingFeatures);
        this->properties.ImageUsage     = CompositeImageFlags(this->imageUsageFlags);
        this->properties.PreTransform   = SelectTransform(transform, surfaceCapabilities.supportedTransforms, surfaceCapabilities.currentTransform);
        this->properties.CompositeAlpha = SelectCompositeAlpha(VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR, surfaceCapabilities.supportedCompositeAlpha);

        this->properties.OldSwapchain   = oldSwapchain.handle;
        this->properties.PresentMode    = presentMode;
    }

    Swapchain::~Swapchain()
    {
        Vulkan::IfNotNullThen<VkSwapchainKHR>(vkDestroySwapchainKHR, device.Handle(), handle);
    }

    void Swapchain::Create()
    {
        // Revalidate the present mode and surface format
        properties.PresentMode   = SelectPresentMode(properties.PresentMode, presentModes, presentModePriorities);
        properties.SurfaceFormat = SelectSurfaceFormat(properties.SurfaceFormat, surfaceFormats, surfaceFormatPriorities);

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.pNext            = nullptr;
        createInfo.minImageCount    = properties.ImageCount;
        createInfo.imageExtent      = properties.Extent;
        createInfo.presentMode      = properties.PresentMode;
        createInfo.imageFormat      = properties.SurfaceFormat.format;
        createInfo.imageColorSpace  = properties.SurfaceFormat.colorSpace;
        createInfo.imageArrayLayers = properties.ArrayLayers;
        createInfo.imageUsage       = properties.ImageUsage;
        createInfo.preTransform     = properties.PreTransform;
        createInfo.compositeAlpha   = properties.CompositeAlpha;
        createInfo.oldSwapchain     = properties.OldSwapchain;
        createInfo.surface          = surface;
        createInfo.clipped          = VK_TRUE; // Get the best performance by enabling clipping.

        Check(vkCreateSwapchainKHR(device.Handle(), &createInfo, nullptr, &handle));

        UINT32 imageAvailable{ 0U };
        Check(vkGetSwapchainImagesKHR(device.Handle(), handle, &imageAvailable, nullptr));

        images.resize(imageAvailable);
        Check(vkGetSwapchainImagesKHR(device.Handle(), handle, &imageAvailable, images.data()));
    }
}
}
