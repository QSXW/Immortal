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
        LOG::WARN<false>("(Swapchain) Image extent ({0}, {1}) not supported. Selecting ({2}, {3}).",
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
                LOG::WARN<false>("(Swapchain) Surface format ({0}) not supported. Selecting ({1}).",
                            Stringify(request), Stringify(*it));
                return *it;
            }
        }
    }
    else
    {
        LOG::DEBUG<false>("(Swapchain) Surface format selected: {0}", Stringify(request));
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
            LOG::WARN<false>("(Swapchain) Image usage ({0}) requested but not supported.", Stringify(flag));
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

    LOG::WARN<false>("(Swapchain) Surface transform '{0}' not supported. Selecting '{1}'.", Stringify(request), Stringify(current));

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
            LOG::WARN<false>("(Swapchain) Composite alpha '{0}' not supported. Selecting '{1}.",
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
        LOG::WARN<false>("(Swapchain) Present mode '{}' not supported. Selecting '{}'.",
                    Stringify(request), Stringify(chosen));
        return chosen;
    }
    else
    {
        LOG::DEBUG<false>("(Swapchain) Present mode selected: {0}", Stringify(request));
        return *it;
    }
}

Swapchain::Swapchain(Device *device, const VkExtent2D &extent, const uint32_t imageCount, const VkSurfaceTransformFlagBitsKHR transform, const VkPresentModeKHR presentMode, VkImageUsageFlags imageUsageFlags) :
    device{ device },
    handle{},
    properties{}
{
    properties.sType         = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    properties.imageExtent   = extent;
    properties.minImageCount = imageCount;
    properties.preTransform  = transform;
    properties.presentMode   = presentMode;
    properties.imageUsage    = imageUsageFlags;
}

Swapchain::Swapchain(Swapchain &oldSwapchain, Device *device, const VkExtent2D &extent, const uint32_t imageCount, const VkSurfaceTransformFlagBitsKHR  transform, const VkPresentModeKHR presentMode, VkImageUsageFlags imageUsageFlags) :
    Swapchain{ device }
{
    auto surface = device->GetSurface();

    PhysicalDevice *physicalDevice = &device->Get<PhysicalDevice &>();
    VkSurfaceCapabilitiesKHR surfaceCapabilities{};
    Check(device->GetSurfaceCapabilities(&surfaceCapabilities));

    std::vector<VkSurfaceFormatKHR> surfaceFormats;
    uint32_t surfaceFormatCount = 0;
    Check(physicalDevice->GetSurfaceFormatsKHR(surface, &surfaceFormatCount, nullptr));

    surfaceFormats.resize(surfaceFormatCount);
    Check(physicalDevice->GetSurfaceFormatsKHR(surface, &surfaceFormatCount, surfaceFormats.data()));

    LOG::DEBUG("Surface supports the following surface formats:");
    for (auto &f : surfaceFormats)
    {
        LOG::DEBUG("  \t{0}", Stringify(f));
    }

    std::vector<VkPresentModeKHR> presentModes{};
    uint32_t presentModeCount = 0;
    Check(physicalDevice->GetSurfacePresentModesKHR(surface, &presentModeCount, nullptr));
    presentModes.resize(presentModeCount);
    Check(physicalDevice->GetSurfacePresentModesKHR( surface, &presentModeCount, presentModes.data()));
    properties.presentMode = SelectPresentMode(properties.presentMode, presentModes, priorities.PresentMode);

    LOG::DEBUG("Surface supports the following present modes:");
    for (auto &p : presentModes)
    {
        LOG::DEBUG("  \t{0}", Stringify(p));
    }

    properties.minImageCount    = SelectImageCount(imageCount, surfaceCapabilities.minImageCount, surfaceCapabilities.maxImageCount);
    properties.imageExtent      = SelectExtent(extent, surfaceCapabilities.minImageExtent, surfaceCapabilities.maxImageExtent, surfaceCapabilities.currentExtent);
    properties.imageArrayLayers = SelectImageArrayLayers(1U, surfaceCapabilities.maxImageArrayLayers);

    VkSurfaceFormatKHR surfaceFormat = {
        .format     = properties.imageFormat,
        .colorSpace = properties.imageColorSpace,
    };

    auto [format, colorSpace] = SelectSurfaceFormat(surfaceFormat, surfaceFormats, priorities.SurfaceFormat);
    properties.imageFormat     = format;
    properties.imageColorSpace = colorSpace;

    VkFormatProperties formatProperties;
    physicalDevice->GetFormatProperties(properties.imageFormat, &formatProperties);
    properties.imageUsage     = SelectImageUsage(imageUsageFlags, surfaceCapabilities.supportedUsageFlags, formatProperties.optimalTilingFeatures);
    properties.preTransform   = SelectTransform(transform, surfaceCapabilities.supportedTransforms, surfaceCapabilities.currentTransform);
    properties.compositeAlpha = SelectCompositeAlpha(VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR, surfaceCapabilities.supportedCompositeAlpha);

    handle   = oldSwapchain.handle;
    Invalidate(properties.imageExtent);
}

Swapchain::~Swapchain()
{
    Destroy();
}

void Swapchain::Destroy()
{
    if (handle)
    {
        device->DestroyAsync(handle);
        handle = VK_NULL_HANDLE;
    }
}

void Swapchain::Invalidate(VkExtent2D extent)
{
    properties.imageExtent = extent;
    properties.oldSwapchain = handle;

    properties.surface = device->GetSurface();
    properties.clipped = VK_TRUE; // Get the best performance by enabling clipping.

    device->Create(&properties, &handle);

    uint32_t count = 0;
    Check(device->GetSwapchainImagesKHR(handle, &count, nullptr));

    images.resize(count);
    Check(device->GetSwapchainImagesKHR(handle, &count, images.data()));

    if (properties.oldSwapchain)
    {
        device->DestroyAsync(properties.oldSwapchain);
    }
}

}
}
