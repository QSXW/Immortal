#pragma once

#include "Core.h"
#include "Graphics/Swapchain.h"

#include "Common.h"
#include "Device.h"
#include "Surface.h"
#include "Semaphore.h"
#include "RenderTarget.h"

namespace Immortal
{

class Window;
namespace Vulkan
{

class RenderTarget;
class Swapchain : public SuperSwapchain, public Handle<VkSwapchainKHR>
{
public:
	VKCPP_SWAPPABLE(Swapchain)

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
                VK_PRESENT_MODE_FIFO_KHR,
                VK_PRESENT_MODE_MAILBOX_KHR,
                VK_PRESENT_MODE_IMMEDIATE_KHR,
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

public:
	Swapchain(Device *device, Surface &&surface, Format format, const VkExtent2D &extent, const uint32_t imageCount, SwapchainMode mode);

    Swapchain(Device                              *device      = nullptr,
              Surface                            &&surface     = {},
              VkFormat                             format      = VK_FORMAT_B8G8R8A8_UNORM,
              const VkExtent2D                    &extent      = {},
              const uint32_t                       imageCount  = 3,
              const VkPresentModeKHR               presentMode = VK_PRESENT_MODE_FIFO_KHR,
              VkImageUsageFlags                    imageUsage  = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
              const VkSurfaceTransformFlagBitsKHR  transform   = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR);

    Swapchain(Swapchain                          &&oldSwapchain,
              Device                              *device,
              Surface                            &&surface,
              Format                               format,
              const VkExtent2D                    &extent      = {},
              const uint32_t                       imageCount  = 3,
              const VkPresentModeKHR               presentMode = VK_PRESENT_MODE_FIFO_KHR,
              VkImageUsageFlags                    imageUsage  = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
              const VkSurfaceTransformFlagBitsKHR  transform   = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR);

    virtual ~Swapchain() override;

    virtual void PrepareNextFrame() override;

	virtual void Resize(uint32_t width, uint32_t height) override;

	virtual SuperRenderTarget *GetCurrentRenderTarget() override;

public:
    void Invalidate(VkExtent2D extent);

    void Destroy();

    void OnPresent();

public:
    using Images = LightArray<VkImage, 3>;

    void Swap(Swapchain &other)
    {
		Handle::Swap(other);
		std::swap(surface,       other.surface      );
		std::swap(device,        other.device       );
		std::swap(renderTargets, other.renderTargets);
		std::swap(properties,    other.properties   );
    }

    VkResult AcquireNextImage(uint32_t *index, VkSemaphore semaphore, VkFence fence)
    {
        return vkAcquireNextImageKHR(*device, handle, std::numeric_limits<uint32_t>::max(), semaphore, fence, index);
    }

    uint32_t GetCurrentBufferIndex() const
    {
		return bufferIndex;
    }

    VkSemaphore GetAcquiredImageReadySemaphore() const
    {
		return semaphores[syncPoint];
    }

public:
    static bool IsValidExtent(const VkExtent2D &extent)
    {
        return extent.width > 0 && extent.height > 0 && extent.width < 0xFFFFFFFF && extent.height < 0xFFFFFFFF;
    }

protected:
    Device *device{ nullptr };

    Surface surface;

    std::vector<URef<RenderTarget>> renderTargets;

    VkSwapchainCreateInfoKHR properties;

    uint32_t bufferIndex;

    uint32_t syncPoint;

    std::vector<Semaphore> semaphores;
};

}
}
