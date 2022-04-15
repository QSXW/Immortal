#pragma once

#include "Render/RenderContext.h"

#include "Device.h"
#include "Instance.h"
#include "Swapchain.h"
#include "RenderTarget.h"
#include "RenderPass.h"
#include "Framebuffer.h"
#include "CommandBuffer.h"

namespace Immortal
{

class Window;
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

    virtual ~RenderContext();

    virtual bool HasSwapchain() override
    {
        return !!swapchain;
    }

    virtual GuiLayer *CreateGuiLayer() override;

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
        if constexpr (IsPrimitiveOf<Extent2D, T>())
        {
            return surfaceExtent;
        }
        if constexpr (IsPrimitiveOf<VkFormat, T>())
        {
            return swapchain->Get<VkFormat>();
        }
    }

    template <class T>
    T *GetAddress()
    {
        if constexpr (IsPrimitiveOf<Device, T>())
        {
            return device;
        }
        if constexpr (IsPrimitiveOf<Framebuffer, T>())
        {
            return GetFramebuffer();
        }
        if constexpr (IsPrimitiveOf<RenderPass, T>())
        {
            return renderPass.get();
        }
        if constexpr (IsPrimitiveOf<Queue, T>())
        {
            return queue;
        }
        if constexpr (IsPrimitiveOf<Swapchain, T>())
        {
            return swapchain.get();
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
        return present.renderTargets[present.bufferIndex]->GetAddress<Framebuffer>();
    }

    CommandBuffer *GetCommandBuffer()
    {
        return present.commandBuffers[present.bufferIndex];
    }

    constexpr size_t FrameSize()
    {
        return Swapchain::MaxFrameCount;
    }

    template <class T>
    void Record(T &&process = [](auto) -> void {}, CommandBuffer::Usage usage = CommandBuffer::Usage::OneTimeSubmit)
    {
        process(GetCommandBuffer());
    }

    template <class T>
    void Submit(T &&process = [](auto) -> void {})
    {
        process(GetCommandBuffer());
    }

private:
    Window *window{ nullptr };

    std::unique_ptr<Instance> instance;
    
    MonoRef<Device> device;

    std::unique_ptr<Swapchain> swapchain;

    Queue *queue{ nullptr };

    std::shared_ptr<RenderPass> renderPass;

    struct
    {
        std::array<CommandBuffer*, Swapchain::MaxFrameCount> commandBuffers;

        std::vector<std::unique_ptr<RenderTarget>> renderTargets;

        uint32_t bufferIndex{ 0 };
    } present;

    VkSurfaceKHR surface{ VK_NULL_HANDLE };

    VkExtent2D surfaceExtent{ 0, 0 };

    Swapchain::Properties swapchainProperties;

    VkFormat depthFormat{ VK_FORMAT_UNDEFINED };

    struct
    {
        VkSurfaceTransformFlagBitsKHR preTransform{ VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR };
    } regisry;

    size_t threadCount{ 1 };
};
}
}
