#pragma once

#include "Render/RenderContext.h"

#include "Device.h"
#include "Instance.h"
#include "Swapchain.h"
#include "RenderTarget.h"
#include "RenderFrame.h"

namespace Immortal
{
namespace Vulkan
{
class RenderContext : public Immortal::RenderContext
{
public:
    using Description           = ::Immortal::RenderContext::Description;
    using SurfaceFormatPriority = std::vector<VkSurfaceFormatKHR>;
    using PresentModePriorities = std::vector<VkPresentModeKHR>;
    using Frames                = std::vector<Unique<RenderFrame>>;

public:
    RenderContext() = default;

    RenderContext(Description &desc);

    ~RenderContext();

    virtual void Init() override;

    virtual void SwapBuffers() override;

    virtual Device *GetDevice() override
    {
        return device.get();
    }

    virtual bool HasSwapchain()
    {
        return !!swapchain;
    }

    void CreateSurface();

public:
    void AddDeviceExtension(const char *extension, bool optional = false)
    {
        DeviceExtensions[extension] = optional;
    }

public:
    void Prepare(size_t threadCount = 1);

    template <class T>
    inline constexpr T &Get()
    {
        if constexpr (typeof<T, Swapchain>())
        {
            return *swapchain;
        }
        if constexpr (typeof<T, Device>())
        {
            return *device;
        }
        if constexpr (typeof<T, Frames>())
        {
            return frames;
        }
        if constexpr (typeof<T, Extent2D>())
        {
            return surfaceExtent;
        }
    }

    template <class T>
    inline constexpr void Set(const T &value)
    {
        if constexpr (std::is_same_v<T, SurfaceFormatPriority>)
        {
            SLASSERT(!value.empty() && "Priority cannot be empty");
            surfaceFormatPriorities = value;
        }
        if constexpr (std::is_same_v<T, VkFormat>)
        {
            if (swapchain)
            {
                swapchain->Get<Swapchain::Properties>().SurfaceFormat.format = value;
            }
        }
        if constexpr (std::is_same_v<T, PresentModePriorities>)
        {
            SLASSERT(!value.empty() && "Priority cannot be empty");
            presentModePriorities = value;
        }
        if constexpr (std::is_same_v<T, VkPresentModeKHR>)
        {
            if (swapchain)
            {
                swapchain->Get<Swapchain::Properties>().PresentMode = value;
            }
        }
    }

private:
    void *handle;

    Unique<Instance>  instance;

    Unique<Device>    device;

    Unique<Swapchain> swapchain;

    VkSurfaceKHR   surface;

    VkExtent2D surfaceExtent;

    const Queue *queue;

    Swapchain::Properties swapchainProperties;

    std::vector<VkPresentModeKHR> presentModePriorities = {
        VK_PRESENT_MODE_MAILBOX_KHR,
        VK_PRESENT_MODE_FIFO_KHR,
        VK_PRESENT_MODE_IMMEDIATE_KHR
    };

    std::vector<VkSurfaceFormatKHR> surfaceFormatPriorities = {
        { VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR },
        { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR },
        { VK_FORMAT_R8G8B8A8_SRGB,  VK_COLOR_SPACE_SRGB_NONLINEAR_KHR },
        { VK_FORMAT_B8G8R8A8_SRGB,  VK_COLOR_SPACE_SRGB_NONLINEAR_KHR }
    };

    Frames frames;

    size_t threadCount{ 1 };

    bool status{ false };

public:
    static VkResult Status;
    static std::unordered_map<const char *, bool> InstanceExtensions;
    static std::unordered_map<const char *, bool> DeviceExtensions;
};
}
}
