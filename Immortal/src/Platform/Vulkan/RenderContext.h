#pragma once

#include "Render/RenderContext.h"

#include "Device.h"
#include "Instance.h"
#include "Swapchain.h"
#include "RenderTarget.h"
#include "RenderFrame.h"
#include "RenderPass.h"
#include "Framebuffer.h"

namespace Immortal
{
namespace Vulkan
{
class RenderContext : public SuperRenderContext
{
public:
    using Super                 = SuperRenderContext;
    using Description           = SuperRenderContext::Description;
    using SurfaceFormatPriority = std::vector<VkSurfaceFormatKHR>;
    using PresentModePriorities = std::vector<VkPresentModeKHR>;
    using Frames                = std::vector<std::unique_ptr<RenderFrame>>;

public:
    RenderContext() = default;

    RenderContext(const Description &desc);

    ~RenderContext();

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
    inline constexpr T Get()
    {
        if constexpr (is_same<T, VkInstance>())
        {
            return instance->Handle();
        }
        if constexpr (is_same<T, VkPhysicalDevice>())
        {
            return instance->SuitablePhysicalDevice().Handle();
        }
        if constexpr (is_same<T, VkDevice>())
        {
            return device->Get<VkDevice>();
        }
        if constexpr (is_same<T, VkQueue>())
        {
            return queue->Handle();
        }
        if constexpr (is_same<T, Queue>())
        {
            return *queue;
        }
        if constexpr (is_same<T, Queue*>())
        {
            return queue;
        }
        if constexpr (is_same<T, Swapchain &>())
        {
            return *swapchain;
        }
        if constexpr (is_same<T, Swapchain *>())
        {
            return swapchain.get();
        }
        if constexpr (is_same<T, Device&>())
        {
            return *device;
        }
        if constexpr (is_same<T, Device*>())
        {
            return device.get();
        }
        if constexpr (is_same<T, Frames&>())
        {
            return frames;
        }
        if constexpr (is_same<T, Extent2D>())
        {
            return surfaceExtent;
        }
        if constexpr (is_same<T, VkFormat>())
        {
            return swapchain->Get<VkFormat>();
        }
        if constexpr (is_same<T, Semaphores>())
        {
            return semaphores;
        }
        if constexpr (is_same<T, RenderPass *>())
        {
            return renderPass.get();
        }
        if constexpr (is_same<T, CommandBuffers *>())
        {
            return &commandBuffers;
        }
    }

    template <class T>
    inline constexpr void Set(const T &value)
    {
        if constexpr (is_same<T, SurfaceFormatPriority>())
        {
            SLASSERT(!value.empty() && "Priority cannot be empty");
            surfaceFormatPriorities = value;
        }
        if constexpr (is_same<T, VkFormat>())
        {
            if (swapchain)
            {
                swapchain->Get<Swapchain::Properties>().SurfaceFormat.format = value;
            }
        }
        if constexpr (is_same<T, PresentModePriorities>())
        {
            SLASSERT(!value.empty() && "Priority cannot be empty");
            presentModePriorities = value;
        }
        if constexpr (is_same<T, VkPresentModeKHR>())
        {
            if (swapchain)
            {
                swapchain->Get<Swapchain::Properties>().PresentMode = value;
            }
        }
        if constexpr (is_same<T, CommandBuffers>())
        {
            commandBuffers = value;
        }
    }

    Framebuffer *GetFramebuffer(size_t index)
    {
        return framebuffers[index].get();
    }

    size_t FrameSize()
    {
        return framebuffers.size();
    }

    Swapchain *UpdateSurface();

    void UpdateSwapchain(const VkExtent2D &extent, const VkSurfaceTransformFlagBitsKHR transform);

private:
    void *handle;

    std::unique_ptr<Instance> instance;

    std::unique_ptr<Device> device;

    std::unique_ptr<RenderPass> renderPass;

    std::vector<std::unique_ptr<Framebuffer>> framebuffers;

    std::unique_ptr<Swapchain> swapchain;

    std::vector<CommandBuffer*> commandBuffers;

    VkSurfaceKHR surface{ VK_NULL_HANDLE };

    VkExtent2D surfaceExtent{ 0, 0 };

    Queue *queue{ nullptr };

    Swapchain::Properties swapchainProperties;

    VkFormat depthFormat{ VK_FORMAT_UNDEFINED };

    Frames frames;

    size_t threadCount{ 1 };

    VkPipelineCache pipelineCache{ VK_NULL_HANDLE };

    VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };

    bool status{ false };

    std::vector<VkPresentModeKHR> presentModePriorities = {
        VK_PRESENT_MODE_MAILBOX_KHR,
        VK_PRESENT_MODE_FIFO_KHR,
        VK_PRESENT_MODE_IMMEDIATE_KHR
    };

    std::vector<VkSurfaceFormatKHR> surfaceFormatPriorities = {
        { 
            VK_FORMAT_R8G8B8A8_UNORM,
            VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
        },
        { 
            VK_FORMAT_B8G8R8A8_UNORM,
            VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
        },
        { 
            VK_FORMAT_R8G8B8A8_SRGB,
            VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
        },
        {
            VK_FORMAT_B8G8R8A8_SRGB,
            VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
        }
    };

    struct
    {
        VkSurfaceTransformFlagBitsKHR preTransform{ VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR };
    } regisry;

public:
    static VkResult Status;
    static std::unordered_map<const char *, bool> InstanceExtensions;
    static std::unordered_map<const char *, bool> DeviceExtensions;
};
}
}
