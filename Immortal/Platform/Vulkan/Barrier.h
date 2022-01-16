#pragma once

#include "Common.h"

namespace Immortal
{
namespace Vulkan
{

struct ImageBarrier : public VkImageMemoryBarrier
{
    using Primitive = VkImageMemoryBarrier;

    ImageBarrier()
    {
        Primitive::sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        Primitive::pNext = nullptr;
        Primitive::srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        Primitive::dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    }

    ImageBarrier(VkImage image,
        VkImageSubresourceRange subresourceRange,
        VkImageLayout oldLayout,
        VkImageLayout newLayout,
        VkAccessFlags srcAccessMask,
        VkAccessFlags dstAccessMask,
        uint32_t srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        uint32_t dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED)
    {
        Primitive::sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        Primitive::pNext = nullptr;
        Primitive::image = image;

        Primitive::oldLayout = oldLayout;
        Primitive::newLayout = newLayout;

        Primitive::srcAccessMask = srcAccessMask;
        Primitive::dstAccessMask = dstAccessMask;

        Primitive::subresourceRange = subresourceRange;
        Primitive::srcQueueFamilyIndex = srcQueueFamilyIndex;
        Primitive::dstQueueFamilyIndex = dstQueueFamilyIndex;
    }

    void Bind(VkImage image)
    {
        Primitive::image = image;
    }

    void Transition(VkImageLayout oldlayout, VkImageLayout newLayout)
    {
        Primitive::oldLayout = oldLayout;
        Primitive::newLayout = newLayout;
    }

    void Transition(VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask)
    {
        Primitive::srcAccessMask = srcAccessMask;
        Primitive::dstAccessMask = dstAccessMask;
    }

    void Swap()
    {
        std::swap(Primitive::oldLayout, Primitive::newLayout);
        std::swap(Primitive::srcAccessMask, Primitive::dstAccessMask);
        std::swap(Primitive::srcQueueFamilyIndex, Primitive::dstQueueFamilyIndex);
    }

    void To(VkAccessFlags dst)
    {
        Primitive::dstAccessMask = dst;
    }

    void To(VkImageLayout dst)
    {
        Primitive::newLayout = dst;
    }
};

}
}
