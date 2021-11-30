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

    static VkResult Status;
    static VkDescriptorSetLayout DescriptorSetLayout;
    static std::unordered_map<const char *, bool> InstanceExtensions;
    static std::unordered_map<const char *, bool> DeviceExtensions;

    static inline RenderContext *That = nullptr;

    void EnableGlobal()
    {
        That = this;
    }

public:
    RenderContext() = default;

    RenderContext(const Description &desc);

    ~RenderContext();

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

    void SetupDescriptorSetLayout();

    template <class T>
    inline constexpr T Get()
    {
        if constexpr (IsPrimitiveOf<Instance, T>())
        {
            return *instance;
        }
        if constexpr (IsPrimitiveOf<PhysicalDevice, T>())
        {
            return instance->SuitablePhysicalDevice();
        }
        if constexpr (IsPrimitiveOf<Device, T>())
        {
            return *device;
        }
        if constexpr (IsPrimitiveOf<Queue, T>())
        {
            return *queue;
        }
        if constexpr (IsPrimitiveOf<Swapchain, T>())
        {
            return *swapchain;
        }
        if constexpr (IsPointerOf<Queue, T>())
        {
            return queue;
        }
        if constexpr (IsPointerOf<Swapchain, T>())
        {
            return swapchain.get();
        }
        if constexpr (IsPointerOf<Device, T>())
        {
            return device.get();
        }
        if constexpr (IsPrimitiveOf<Extent2D, T>())
        {
            return surfaceExtent;
        }
        if constexpr (IsPrimitiveOf<VkFormat, T>())
        {
            return swapchain->Get<VkFormat>();
        }
        if constexpr (IsPointerOf<RenderPass, T>())
        {
            return renderPass.get();
        }
    }

    template <class T>
    inline constexpr void Set(const T value)
    {
        if constexpr (IsPrimitiveOf<SurfaceFormatPriority, T>())
        {
            SLASSERT(!value.empty() && "Priority cannot be empty");
            surfaceFormatPriorities = value;
        }
        if constexpr (IsPrimitiveOf<VkFormat, T>())
        {
            if (swapchain)
            {
                swapchain->Get<Swapchain::Properties>().SurfaceFormat.format = value;
            }
        }
        if constexpr (IsPrimitiveOf<PresentModePriorities, T>())
        {
            SLASSERT(!value.empty() && "Priority cannot be empty");
            presentModePriorities = value;
        }
        if constexpr (IsPrimitiveOf<VkPresentModeKHR, T>())
        {
            if (swapchain)
            {
                swapchain->Get<Swapchain::Properties>().PresentMode = value;
            }
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

    const Framebuffer *GetFramebuffer()
    {
        return present.renderTargets[present.bufferIndex]->GetAddress<Framebuffer>();
    }

    CommandBuffer *GetCommandBuffer()
    {
        return present.commandBuffers[present.bufferIndex].get();
    }

    constexpr size_t FrameSize()
    {
        return Swapchain::MaxFrameCount;
    }

    template <class T>
    void Record(T &&process = [](auto, auto)->void {}, CommandBuffer::Usage usage = CommandBuffer::Usage::OneTimeSubmit)
    {
        VkRenderPassBeginInfo beginInfo = {};
        beginInfo.sType       = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        beginInfo.pNext       = nullptr;
        beginInfo.renderPass  = *renderPass;
        beginInfo.framebuffer = *GetFramebuffer();
        process(GetCommandBuffer(), &beginInfo);
    }
    
    template <class T>
    void Begin(T &&postProcess = [](auto) -> void {}, CommandBuffer::Usage usage = CommandBuffer::Usage::OneTimeSubmit)
    {
        postProcess(GetCommandBuffer());
    }

    template <class T>
    void End(T &&preprocess = [](auto) -> void {})
    {
        preprocess(GetCommandBuffer());
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

    Queue *queue{ nullptr };

    std::shared_ptr<RenderPass> renderPass;

    struct
    {
        std::array<std::unique_ptr<CommandBuffer>, Swapchain::MaxFrameCount> commandBuffers;

        std::vector<std::unique_ptr<RenderTarget>> renderTargets;

        uint32_t bufferIndex{ 0 };
    } present;

    VkSurfaceKHR surface{ VK_NULL_HANDLE };

    VkExtent2D surfaceExtent{ 0, 0 };

    Swapchain::Properties swapchainProperties;

    VkFormat depthFormat{ VK_FORMAT_UNDEFINED };

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
};
}
}
