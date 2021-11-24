#pragma once

#include "Render/RenderContext.h"

#include "Device.h"
#include "Instance.h"
#include "Swapchain.h"
#include "RenderTarget.h"
#include "RenderFrame.h"
#include "RenderPass.h"
#include "Framebuffer.h"
#include "CommandBuffer.h"

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

public:
    void CreateSurface();

    Swapchain *UpdateSurface();
    
    void Prepare(size_t threadCount = 1);

    void UpdateSwapchain(const VkExtent2D &extent, const VkSurfaceTransformFlagBitsKHR transform);

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

    void AddDeviceExtension(const char *extension, bool optional = false)
    {
        DeviceExtensions[extension] = optional;
    }

    void MoveToFrame(uint32_t index)
    {
        present.bufferIndex = index;
    }

    Framebuffer *GetFramebuffer()
    {
        return present.framebuffers[present.bufferIndex].get();
    }

    CommandBuffer *GetCommandBuffer()
    {
        return present.commandBuffers[present.bufferIndex].get();
    }

    size_t FrameSize()
    {
        return present.framebuffers.size();
    }

    template <class T>
    void Record(T &&process = [](auto, auto)->void {}, CommandBuffer::Usage usage = CommandBuffer::Usage::OneTimeSubmit)
    {
        auto cmdbuf = GetCommandBuffer();
        Check(cmdbuf->Begin(usage));
        {
            VkRenderPassBeginInfo beginInfo = {};
            beginInfo.sType                    = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            beginInfo.pNext                    = nullptr;
            beginInfo.renderPass               = nullptr;
            beginInfo.framebuffer              = *GetFramebuffer();
            process(cmdbuf, &beginInfo);
        }
        Check(cmdbuf->End());
    }
    
    template <class T>
    void Begin(T &&postProcess = [](auto) -> void {}, CommandBuffer::Usage usage = CommandBuffer::Usage::OneTimeSubmit)
    {
        auto cmdBuf = GetCommandBuffer();
        Check(cmdBuf->Begin(usage));
        postProcess(cmdBuf);
    }

    template <class T>
    void End(T &&preprocess = [](auto) -> void {})
    {
        auto cmdbuf = GetCommandBuffer();
        preprocess(cmdbuf);
        Check(cmdbuf->End());
    }

    template <class T>
    void Submit(T &&process = [](auto) -> void {})
    {
        process(GetCommandBuffer());
    }

private:
    void *handle{ nullptr };

    std::unique_ptr<Instance> instance;

    std::unique_ptr<Device> device;

    std::unique_ptr<Swapchain> swapchain;

    std::shared_ptr<RenderPass> renderPass;

    Queue *queue{ nullptr };

    struct
    {
        uint32_t bufferIndex{ 0 };
        std::vector<std::unique_ptr<Framebuffer>> framebuffers;
        std::array<std::unique_ptr<CommandBuffer>, Swapchain::MaxFrameCount> commandBuffers;
    } present;

    VkSurfaceKHR surface{ VK_NULL_HANDLE };

    VkExtent2D surfaceExtent{ 0, 0 };

    Swapchain::Properties swapchainProperties;

    VkFormat depthFormat{ VK_FORMAT_UNDEFINED };

    Frames frames;

    VkPipelineCache pipelineCache{ VK_NULL_HANDLE };

    struct
    {
        std::vector<VkPresentModeKHR> presentMode = {
            VK_PRESENT_MODE_MAILBOX_KHR,
            VK_PRESENT_MODE_FIFO_KHR,
            VK_PRESENT_MODE_IMMEDIATE_KHR
        };

        std::vector<VkSurfaceFormatKHR> surfaceFormat = {
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
    } priorities;

    struct
    {
        VkSurfaceTransformFlagBitsKHR preTransform{ VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR };
    } regisry;

    size_t threadCount{ 1 };

    bool status{ false };

public:
    static VkResult Status;
    static std::unordered_map<const char *, bool> InstanceExtensions;
    static std::unordered_map<const char *, bool> DeviceExtensions;
};
}
}
