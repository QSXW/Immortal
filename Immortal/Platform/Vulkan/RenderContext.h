#pragma once

#include "Render/RenderContext.h"

#include "Device.h"
#include "Instance.h"
#include "Swapchain.h"
#include "RenderTarget.h"
#include "RenderPass.h"
#include "Framebuffer.h"
#include "CommandBuffer.h"
#include "GuiLayer.h"

namespace Immortal
{

class Window;
namespace Vulkan
{

template <class T>
concept ExposedType = (
    std::is_same_v<T, Instance>       || std::is_same_v<T, Instance&>       ||
    std::is_same_v<T, PhysicalDevice> || std::is_same_v<T, PhysicalDevice&> ||
    std::is_same_v<T, Device>         || std::is_same_v<T, Device&>         ||
    std::is_same_v<T, Queue>          || std::is_same_v<T, Queue&>          ||
    std::is_same_v<T, RenderPass>     || std::is_same_v<T, RenderPass&>     ||
    std::is_same_v<T, RenderTarget>   || std::is_same_v<T, RenderTarget&>   ||
    std::is_same_v<T, Swapchain>      || std::is_same_v<T, Swapchain&>      ||
    std::is_same_v<T, Extent2D>       || std::is_same_v<T, Extent2D&>       
    );

class RenderContext : public SuperRenderContext
{
public:
    using Super                 = SuperRenderContext;
    using Description           = SuperRenderContext::Description;
    using SurfaceFormatPriority = std::vector<VkSurfaceFormatKHR>;
    using PresentModePriorities = std::vector<VkPresentModeKHR>;

    static VkResult Status;
    static VkDescriptorSetLayout DescriptorSetLayout;
    static MonoRef<Sampler> ImmutableSampler;
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

    virtual GuiLayer::Super *CreateGuiLayer() override;

public:
    void CreateSurface();

    Swapchain *UpdateSurface();

    void Prepare(size_t threadCount = 1);

    void UpdateSwapchain(const VkExtent2D &extent, const VkSurfaceTransformFlagBitsKHR transform);

    void SetupDescriptorSetLayout();

    VkExtent2D GetViewport() const
    {
        return surfaceExtent;
    }

    template <ExposedType T>
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
    }

    template <ExposedType T>
    T *GetAddress()
    {
        if constexpr (IsPrimitiveOf<Device, T>())
        {
            return device;
        }
        if constexpr (IsPrimitiveOf<RenderPass, T>())
        {
            return renderPass;
        }
        if constexpr (IsPrimitiveOf<Queue, T>())
        {
            return queue;
        }
        if constexpr (IsPrimitiveOf<Swapchain, T>())
        {
            return swapchain;
        }
        if constexpr (IsPrimitiveOf<RenderTarget, T>())
        {
            return GetRenderTarget();
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

    RenderTarget *GetRenderTarget()
    {
        return present.renderTargets[present.bufferIndex].get();
    }

    Framebuffer *GetFramebuffer()
    {
        return GetRenderTarget()->GetAddress<Framebuffer>();
    }

    constexpr size_t FrameSize()
    {
        return Swapchain::MaxFrameCount;
    }

    template <class T>
    void Record(T &&process = [](CommandBuffer *) -> void {}, CommandBuffer::Usage usage = CommandBuffer::Usage::OneTimeSubmit)
    {
        device->Submit(process);
    }

    template <class T>
    void Submit(T &&process = [](CommandBuffer *) -> void {})
    {
        device->Submit(process);
    }

private:
    Window *window{ nullptr };

    MonoRef<Instance> instance;
    
    MonoRef<Device> device;

    MonoRef<Swapchain> swapchain;

    Queue *queue{ nullptr };

    Ref<RenderPass> renderPass;

    struct
    {
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
