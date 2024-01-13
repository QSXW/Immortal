#include "Common.h"

#define VOLK_IMPLEMENTATION
#include "volk.h"

#ifdef VK_NO_PROTOTYPES
#undef VK_NO_PROTOTYPES
#endif

namespace Immortal
{
namespace Vulkan
{

const char *Stringify(VkFormat format)
{
    switch (format)
    {
        VK_CASE(VK_FORMAT_R4G4_UNORM_PACK8);
        VK_CASE(VK_FORMAT_R4G4B4A4_UNORM_PACK16);
        VK_CASE(VK_FORMAT_B4G4R4A4_UNORM_PACK16);
        VK_CASE(VK_FORMAT_R5G6B5_UNORM_PACK16);
        VK_CASE(VK_FORMAT_B5G6R5_UNORM_PACK16);
        VK_CASE(VK_FORMAT_R5G5B5A1_UNORM_PACK16);
        VK_CASE(VK_FORMAT_B5G5R5A1_UNORM_PACK16);
        VK_CASE(VK_FORMAT_A1R5G5B5_UNORM_PACK16);
        VK_CASE(VK_FORMAT_R8_UNORM);
        VK_CASE(VK_FORMAT_R8_SNORM);
        VK_CASE(VK_FORMAT_R8_USCALED);
        VK_CASE(VK_FORMAT_R8_SSCALED);
        VK_CASE(VK_FORMAT_R8_UINT);
        VK_CASE(VK_FORMAT_R8_SINT);
        VK_CASE(VK_FORMAT_R8_SRGB);
        VK_CASE(VK_FORMAT_R8G8_UNORM);
        VK_CASE(VK_FORMAT_R8G8_SNORM);
        VK_CASE(VK_FORMAT_R8G8_USCALED);
        VK_CASE(VK_FORMAT_R8G8_SSCALED);
        VK_CASE(VK_FORMAT_R8G8_UINT);
        VK_CASE(VK_FORMAT_R8G8_SINT);
        VK_CASE(VK_FORMAT_R8G8_SRGB);
        VK_CASE(VK_FORMAT_R8G8B8_UNORM);
        VK_CASE(VK_FORMAT_R8G8B8_SNORM);
        VK_CASE(VK_FORMAT_R8G8B8_USCALED);
        VK_CASE(VK_FORMAT_R8G8B8_SSCALED);
        VK_CASE(VK_FORMAT_R8G8B8_UINT);
        VK_CASE(VK_FORMAT_R8G8B8_SINT);
        VK_CASE(VK_FORMAT_R8G8B8_SRGB);
        VK_CASE(VK_FORMAT_B8G8R8_UNORM);
        VK_CASE(VK_FORMAT_B8G8R8_SNORM);
        VK_CASE(VK_FORMAT_B8G8R8_USCALED);
        VK_CASE(VK_FORMAT_B8G8R8_SSCALED);
        VK_CASE(VK_FORMAT_B8G8R8_UINT);
        VK_CASE(VK_FORMAT_B8G8R8_SINT);
        VK_CASE(VK_FORMAT_B8G8R8_SRGB);
        VK_CASE(VK_FORMAT_R8G8B8A8_UNORM);
        VK_CASE(VK_FORMAT_R8G8B8A8_SNORM);
        VK_CASE(VK_FORMAT_R8G8B8A8_USCALED);
        VK_CASE(VK_FORMAT_R8G8B8A8_SSCALED);
        VK_CASE(VK_FORMAT_R8G8B8A8_UINT);
        VK_CASE(VK_FORMAT_R8G8B8A8_SINT);
        VK_CASE(VK_FORMAT_R8G8B8A8_SRGB);
        VK_CASE(VK_FORMAT_B8G8R8A8_UNORM);
        VK_CASE(VK_FORMAT_B8G8R8A8_SNORM);
        VK_CASE(VK_FORMAT_B8G8R8A8_USCALED);
        VK_CASE(VK_FORMAT_B8G8R8A8_SSCALED);
        VK_CASE(VK_FORMAT_B8G8R8A8_UINT);
        VK_CASE(VK_FORMAT_B8G8R8A8_SINT);
        VK_CASE(VK_FORMAT_B8G8R8A8_SRGB);
        VK_CASE(VK_FORMAT_A8B8G8R8_UNORM_PACK32);
        VK_CASE(VK_FORMAT_A8B8G8R8_SNORM_PACK32);
        VK_CASE(VK_FORMAT_A8B8G8R8_USCALED_PACK32);
        VK_CASE(VK_FORMAT_A8B8G8R8_SSCALED_PACK32);
        VK_CASE(VK_FORMAT_A8B8G8R8_UINT_PACK32);
        VK_CASE(VK_FORMAT_A8B8G8R8_SINT_PACK32);
        VK_CASE(VK_FORMAT_A8B8G8R8_SRGB_PACK32);
        VK_CASE(VK_FORMAT_A2R10G10B10_UNORM_PACK32);
        VK_CASE(VK_FORMAT_A2R10G10B10_SNORM_PACK32);
        VK_CASE(VK_FORMAT_A2R10G10B10_USCALED_PACK32);
        VK_CASE(VK_FORMAT_A2R10G10B10_SSCALED_PACK32);
        VK_CASE(VK_FORMAT_A2R10G10B10_UINT_PACK32);
        VK_CASE(VK_FORMAT_A2R10G10B10_SINT_PACK32);
        VK_CASE(VK_FORMAT_A2B10G10R10_UNORM_PACK32);
        VK_CASE(VK_FORMAT_A2B10G10R10_SNORM_PACK32);
        VK_CASE(VK_FORMAT_A2B10G10R10_USCALED_PACK32);
        VK_CASE(VK_FORMAT_A2B10G10R10_SSCALED_PACK32);
        VK_CASE(VK_FORMAT_A2B10G10R10_UINT_PACK32);
        VK_CASE(VK_FORMAT_A2B10G10R10_SINT_PACK32);
        VK_CASE(VK_FORMAT_R16_UNORM);
        VK_CASE(VK_FORMAT_R16_SNORM);
        VK_CASE(VK_FORMAT_R16_USCALED);
        VK_CASE(VK_FORMAT_R16_SSCALED);
        VK_CASE(VK_FORMAT_R16_UINT);
        VK_CASE(VK_FORMAT_R16_SINT);
        VK_CASE(VK_FORMAT_R16_SFLOAT);
        VK_CASE(VK_FORMAT_R16G16_UNORM);
        VK_CASE(VK_FORMAT_R16G16_SNORM);
        VK_CASE(VK_FORMAT_R16G16_USCALED);
        VK_CASE(VK_FORMAT_R16G16_SSCALED);
        VK_CASE(VK_FORMAT_R16G16_UINT);
        VK_CASE(VK_FORMAT_R16G16_SINT);
        VK_CASE(VK_FORMAT_R16G16_SFLOAT);
        VK_CASE(VK_FORMAT_R16G16B16_UNORM);
        VK_CASE(VK_FORMAT_R16G16B16_SNORM);
        VK_CASE(VK_FORMAT_R16G16B16_USCALED);
        VK_CASE(VK_FORMAT_R16G16B16_SSCALED);
        VK_CASE(VK_FORMAT_R16G16B16_UINT);
        VK_CASE(VK_FORMAT_R16G16B16_SINT);
        VK_CASE(VK_FORMAT_R16G16B16_SFLOAT);
        VK_CASE(VK_FORMAT_R16G16B16A16_UNORM);
        VK_CASE(VK_FORMAT_R16G16B16A16_SNORM);
        VK_CASE(VK_FORMAT_R16G16B16A16_USCALED);
        VK_CASE(VK_FORMAT_R16G16B16A16_SSCALED);
        VK_CASE(VK_FORMAT_R16G16B16A16_UINT);
        VK_CASE(VK_FORMAT_R16G16B16A16_SINT);
        VK_CASE(VK_FORMAT_R16G16B16A16_SFLOAT);
        VK_CASE(VK_FORMAT_R32_UINT);
        VK_CASE(VK_FORMAT_R32_SINT);
        VK_CASE(VK_FORMAT_R32_SFLOAT);
        VK_CASE(VK_FORMAT_R32G32_UINT);
        VK_CASE(VK_FORMAT_R32G32_SINT);
        VK_CASE(VK_FORMAT_R32G32_SFLOAT);
        VK_CASE(VK_FORMAT_R32G32B32_UINT);
        VK_CASE(VK_FORMAT_R32G32B32_SINT);
        VK_CASE(VK_FORMAT_R32G32B32_SFLOAT);
        VK_CASE(VK_FORMAT_R32G32B32A32_UINT);
        VK_CASE(VK_FORMAT_R32G32B32A32_SINT);
        VK_CASE(VK_FORMAT_R32G32B32A32_SFLOAT);
        VK_CASE(VK_FORMAT_R64_UINT);
        VK_CASE(VK_FORMAT_R64_SINT);
        VK_CASE(VK_FORMAT_R64_SFLOAT);
        VK_CASE(VK_FORMAT_R64G64_UINT);
        VK_CASE(VK_FORMAT_R64G64_SINT);
        VK_CASE(VK_FORMAT_R64G64_SFLOAT);
        VK_CASE(VK_FORMAT_R64G64B64_UINT);
        VK_CASE(VK_FORMAT_R64G64B64_SINT);
        VK_CASE(VK_FORMAT_R64G64B64_SFLOAT);
        VK_CASE(VK_FORMAT_R64G64B64A64_UINT);
        VK_CASE(VK_FORMAT_R64G64B64A64_SINT);
        VK_CASE(VK_FORMAT_R64G64B64A64_SFLOAT);
        VK_CASE(VK_FORMAT_B10G11R11_UFLOAT_PACK32);
        VK_CASE(VK_FORMAT_E5B9G9R9_UFLOAT_PACK32);
        VK_CASE(VK_FORMAT_D16_UNORM);
        VK_CASE(VK_FORMAT_X8_D24_UNORM_PACK32);
        VK_CASE(VK_FORMAT_D32_SFLOAT);
        VK_CASE(VK_FORMAT_S8_UINT);
        VK_CASE(VK_FORMAT_D16_UNORM_S8_UINT);
        VK_CASE(VK_FORMAT_D24_UNORM_S8_UINT);
        VK_CASE(VK_FORMAT_D32_SFLOAT_S8_UINT);
        VK_CASE(VK_FORMAT_UNDEFINED);
    default:
        return "VK_FORMAT_INVALID";
    }
}

const char *Stringify(VkPresentModeKHR presentMode)
{
    switch (presentMode)
    {
        VK_CASE(VK_PRESENT_MODE_MAILBOX_KHR);
        VK_CASE(VK_PRESENT_MODE_IMMEDIATE_KHR);
        VK_CASE(VK_PRESENT_MODE_FIFO_KHR);
        VK_CASE(VK_PRESENT_MODE_FIFO_RELAXED_KHR);;
        VK_CASE(VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR);
        VK_CASE(VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR);
    default:
        return "UNKNOWN_PRESENT_MODE";
    }
}

const char *Stringify(VkImageUsageFlagBits imageUsage)
{
    switch (imageUsage)
    {
        VK_CASE(VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
        VK_CASE(VK_IMAGE_USAGE_TRANSFER_DST_BIT);
        VK_CASE(VK_IMAGE_USAGE_SAMPLED_BIT);
        VK_CASE(VK_IMAGE_USAGE_STORAGE_BIT);
        VK_CASE(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
        VK_CASE(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
        VK_CASE(VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT);
        VK_CASE(VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT);
        VK_CASE(VK_IMAGE_USAGE_FLAG_BITS_MAX_ENUM);
    default:
        return "UNKNOWN IMAGE USAGE FLAG";
    }
}

const char *Stringify(VkSurfaceTransformFlagBitsKHR transform)
{
    switch (transform)
    {
        VK_CASE(VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR);
        VK_CASE(VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR);
        VK_CASE(VK_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR);
        VK_CASE(VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR);
        VK_CASE(VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_BIT_KHR);
        VK_CASE(VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_90_BIT_KHR);
        VK_CASE(VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_180_BIT_KHR);
        VK_CASE(VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_270_BIT_KHR);
        VK_CASE(VK_SURFACE_TRANSFORM_INHERIT_BIT_KHR);
        VK_CASE(VK_SURFACE_TRANSFORM_FLAG_BITS_MAX_ENUM_KHR);
    default:
        return "[Unknown transform flag]";
    }
}

const char *Stringify(VkCompositeAlphaFlagBitsKHR compositeAlpha)
{
    switch (compositeAlpha)
    {
        VK_CASE(VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR);
        VK_CASE(VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR);
        VK_CASE(VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR);
        VK_CASE(VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR);
        VK_CASE(VK_COMPOSITE_ALPHA_FLAG_BITS_MAX_ENUM_KHR);
    default:
        return "UNKNOWN COMPOSITE ALPHA FLAG";
    }
}

VkFormat SuitableDepthFormat(VkPhysicalDevice physicalDevice, bool depthOnly, const std::vector<VkFormat> &depthFormatPriorities)
{
    VkFormat depthFormat{ VK_FORMAT_UNDEFINED };

    for (auto &format : depthFormatPriorities)
    {
        if (depthOnly && IsDepthOnlyFormat(format))
        {
            continue;
        }

        VkFormatProperties properties;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &properties);
        if (properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
        {
            depthFormat = format;
            break;
        }
    }

    if (depthFormat != VK_FORMAT_UNDEFINED)
    {
        LOG::DEBUG<false>("Depth format selected: {0}", Stringify(depthFormat));
        return depthFormat;
    }

    SLASSERT(false && "No suitable depth format could be determined");
    return VK_FORMAT_UNDEFINED;
}

}
}
