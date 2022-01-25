#include "impch.h"
#include "Swapchain.h"

#include "Framework/Utils.h"

namespace Immortal
{
namespace Vulkan
{

static inline constexpr bool SwapchainOutput = false;

inline uint32_t SelectImageCount(uint32_t request, uint32_t min, uint32_t max)
{
    Utils::Clamp(request, min, max);
    return request;
}

inline uint32_t SelectImageArrayLayers(uint32_t request, uint32_t max)
{
    Utils::Clamp(request, 1u, max);
    return request;
}

inline VkExtent2D SelectExtent(VkExtent2D request, const VkExtent2D &min, const VkExtent2D &max, const VkExtent2D &current)
{
    if (request.width < 1 || request.height < 1)
    {
        LOG::WARN<SwapchainOutput>("(Swapchain) Image extent ({0}, {1}) not supported. Selecting ({2}, {3}).",
            request.width, request.height, current.width, current.height);
        return current;
    }

    Utils::Clamp<uint32_t>(request.width, min.width, max.width);
    Utils::Clamp<uint32_t>(request.height, min.height, max.height);
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
            end:
                LOG::WARN<SwapchainOutput>("(Swapchain) Surface format ({0}) not supported. Selecting ({1}).",
                            Stringify(request), Stringify(*it));
                return *it;
            }
        }
        it = available.begin();
        goto end;
    }
    else
    {
        LOG::DEBUG<SwapchainOutput>("(Swapchain) Surface format selected: {0}", Stringify(request));
    }
    return *it;
}

inline VkImageUsageFlags SelectImageUsage(VkImageUsageFlags &request, VkImageUsageFlags support, VkFormatFeatureFlags supportedFeatures)
{
    VkImageUsageFlags validated = request & support;

    if (validated != request)
    {
        VkImageUsageFlagBits flag = VkImageUsageFlagBits(1);
        while (!(flag & request) && !(flag & validated))
        {
            LOG::WARN<SwapchainOutput>("(Swapchain) Image usage ({0}) requested but not supported.", Stringify(flag));
            flag = VkImageUsageFlagBits((int)flag << 1);
        }
    }

    if (!validated)
    {   
        validated = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        validated &= support;
        if (VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT & supportedFeatures)
        {
            validated |= (VK_IMAGE_USAGE_STORAGE_BIT & support);
        }
    }

    THROWIF(!validated, "No compatible image usage found.");

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

    LOG::WARN<SwapchainOutput>("(Swapchain) Surface transform '{0}' not supported. Selecting '{1}'.", Stringify(request), Stringify(current));

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

    for (size_t i = 0; i < SL_ARRAY_LENGTH(compositeAlphaFlags); i++)
    {
        if (compositeAlphaFlags[i] & support)
        {
            LOG::WARN<SwapchainOutput>("(Swapchain) Composite alpha '{0}' not supported. Selecting '{1}.",
                        Stringify(request), Stringify(compositeAlphaFlags[i]));
            return compositeAlphaFlags[i];
        }
    }

    SLASSERT(false && "No compatible composite alpha found.");
    return VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
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
        LOG::WARN<SwapchainOutput>("(Swapchain) Present mode '{}' not supported. Selecting '{}'."),
                    Stringify(request), Stringify(chosen);
        return chosen;
    }
    else
    {
        LOG::DEBUG<SwapchainOutput>("(Swapchain) Present mode selected: {0}"), Stringify(request);
        return *it;
    }
}

Swapchain::Swapchain(Swapchain &oldSwapchain, const VkExtent2D &extent, const VkSurfaceTransformFlagBitsKHR transform) :
    Swapchain{ oldSwapchain, oldSwapchain.device, oldSwapchain.surface, extent, oldSwapchain.properties.ImageCount, transform, oldSwapchain.properties.PresentMode, oldSwapchain.properties.ImageUsage }
{
    presentModePriorities = oldSwapchain.presentModePriorities;
    surfaceFormatPriorities = oldSwapchain.surfaceFormatPriorities;
    Create();
}

Swapchain::Swapchain(Device                               *device,
                     VkSurfaceKHR                         surface,
                     const VkExtent2D &extent,
                     const uint32_t                       imageCount,
                     const VkSurfaceTransformFlagBitsKHR  transform,
                     const VkPresentModeKHR               presentMode,
                           VkImageUsageFlags              imageUsageFlags) :
    Swapchain{ *this, device, surface, extent, imageCount, transform, presentMode, imageUsageFlags }
{

}

Swapchain::Swapchain(Swapchain                           &oldSwapchain,
                     Device                              *device,
                     VkSurfaceKHR                         surface,
                     const VkExtent2D                    &extent,
                     const uint32_t                       imageCount,
                     const VkSurfaceTransformFlagBitsKHR  transform,
                     const VkPresentModeKHR               presentMode,
                           VkImageUsageFlags              imageUsageFlags) :
    device{ device },
    surface{ surface }
{
    presentModePriorities  = oldSwapchain.presentModePriorities;
    surfaceFormatPriorities = oldSwapchain.surfaceFormatPriorities;

    VkPhysicalDevice &physicalDevice = device->Get<PhysicalDevice>();
    VkSurfaceCapabilitiesKHR surfaceCapabilities{};
    Check(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities));

    uint32_t surfaceFormatCount = 0;
    Check(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, nullptr));
    surfaceFormats.resize(surfaceFormatCount);
    Check(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, surfaceFormats.data()));

    LOG::DEBUG<SwapchainOutput>("Surface supports the following surface formats:");
    for (auto &f : surfaceFormats)
    {
        LOG::DEBUG<SwapchainOutput>("  \t{0}", Stringify(f));
    }

    uint32_t presentModeCount = 0;
    Check(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr));
    presentModes.resize(presentModeCount);
    Check(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, presentModes.data()));

    LOG::DEBUG<SwapchainOutput>("Surface supports the following present modes:");
    for (auto &p : presentModes)
    {
        LOG::DEBUG<SwapchainOutput>("  \t{0}", Stringify(p));
    }

    this->properties.ImageCount    = SelectImageCount(imageCount, surfaceCapabilities.minImageCount, surfaceCapabilities.maxImageCount);
    this->properties.Extent        = SelectExtent(extent, surfaceCapabilities.minImageExtent, surfaceCapabilities.maxImageExtent, surfaceCapabilities.currentExtent);
    this->properties.ArrayLayers   = SelectImageArrayLayers(1U, surfaceCapabilities.maxImageArrayLayers);
    this->properties.SurfaceFormat = SelectSurfaceFormat(this->properties.SurfaceFormat, surfaceFormats, surfaceFormatPriorities);

    VkFormatProperties formatProperties;
    vkGetPhysicalDeviceFormatProperties(physicalDevice, this->properties.SurfaceFormat.format, &formatProperties);
    this->properties.ImageUsage     = SelectImageUsage(imageUsageFlags, surfaceCapabilities.supportedUsageFlags, formatProperties.optimalTilingFeatures);
    this->properties.PreTransform   = SelectTransform(transform, surfaceCapabilities.supportedTransforms, surfaceCapabilities.currentTransform);
    this->properties.CompositeAlpha = SelectCompositeAlpha(VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR, surfaceCapabilities.supportedCompositeAlpha);

    this->properties.OldSwapchain   = oldSwapchain.handle;
    this->properties.PresentMode    = presentMode;
}

Swapchain::~Swapchain()
{
    IfNotNullThen<VkSwapchainKHR>(vkDestroySwapchainKHR, *device, handle);
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

    device->Create(&createInfo, nullptr, &handle);

    uint32_t count = 0;
    Check(vkGetSwapchainImagesKHR(*device, handle, &count, nullptr));

    images.resize(count);
    Check(vkGetSwapchainImagesKHR(*device, handle, &count, images.data()));
}

}
}
