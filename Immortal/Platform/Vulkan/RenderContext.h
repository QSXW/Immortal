#pragma once

#include "Render/RenderContext.h"
#include "ImGui/GuiLayer.h"

#include "Device.h"
#include "Swapchain.h"
#include "RenderTarget.h"
#include "RenderPass.h"
#include "Framebuffer.h"
#include "CommandBuffer.h"
#include "Pipeline.h"
#include "SemaphorePool.h"
#include "AccelerationStructure.h"
#include "Surface.h"

namespace Immortal
{

class Window;
namespace Vulkan
{

class Instance;
template <class T>
concept ExposedType = (
    std::is_same_v<T, Instance>       || std::is_same_v<T, Instance &>       ||
    std::is_same_v<T, PhysicalDevice> || std::is_same_v<T, PhysicalDevice &> ||
    std::is_same_v<T, Device>         || std::is_same_v<T, Device &>         ||
    std::is_same_v<T, Queue>          || std::is_same_v<T, Queue &>          ||
    std::is_same_v<T, RenderPass>     || std::is_same_v<T, RenderPass &>     ||
    std::is_same_v<T, RenderTarget>   || std::is_same_v<T, RenderTarget &>   ||
    std::is_same_v<T, Swapchain>      || std::is_same_v<T, Swapchain &>
    );

struct RenderSemaphore
{
	VkSemaphore acquiredImageReady;
	VkSemaphore renderComplete;
};

class RenderContext : public SuperRenderContext
{
public:
    using Super                 = SuperRenderContext;
    using Description           = SuperRenderContext::Description;
    using SurfaceFormatPriority = std::vector<VkSurfaceFormatKHR>;
    using PresentModePriorities = std::vector<VkPresentModeKHR>;

    static VkResult Status;
    static VkDescriptorSetLayout DescriptorSetLayout;
    static URef<Sampler> ImmutableSampler;
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

    virtual SuperGuiLayer *CreateGuiLayer() override;

    virtual void SwapBuffers() override;

	virtual void PrepareFrame() override;

	virtual void OnResize(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;

	virtual SuperShader *CreateShader(const std::string &filepath, Shader::Type type) override;

	virtual SuperGraphicsPipeline *CreateGraphicsPipeline(Ref<Shader::Super> shader) override;

	virtual SuperComputePipeline *CreateComputePipeline(Shader::Super *shader) override;

	virtual SuperTexture *CreateTexture(const std::string &filepath, const Texture::Description &description = {}) override;

	virtual SuperTexture *CreateTexture(uint32_t width, uint32_t height, const void *data, const Texture::Description &description) override;

	virtual SuperTextureCube *CreateTextureCube(uint32_t width, uint32_t height, const Texture::Description &description) override;

	virtual SuperBuffer *CreateBuffer(const size_t size, const void *data, Buffer::Type type) override;

	virtual SuperBuffer *CreateBuffer(const size_t size, Buffer::Type type) override;

	virtual SuperBuffer *CreateBuffer(const size_t size, uint32_t binding) override;

	virtual SuperRenderTarget *CreateRenderTarget(const RenderTarget::Description &description) override;

	virtual DescriptorBuffer *CreateImageDescriptor(uint32_t count) override;

	virtual DescriptorBuffer *CreateBufferDescriptor(uint32_t count) override;

    virtual AccelerationStructure *CreateAccelerationStructure(const SuperBuffer *pVertexBuffer, const InputElementDescription &desc, const SuperBuffer *pIndexBuffer, const SuperBuffer *pTranformBuffer) override;

	virtual void PushConstant(SuperGraphicsPipeline *pipeline, Shader::Stage stage, uint32_t size, const void *data, uint32_t offset) override;

	virtual void PushConstant(SuperComputePipeline *pipeline, uint32_t size, const void *data, uint32_t offset) override;

	virtual void Draw(SuperGraphicsPipeline *pipeline) override;

	virtual void Dispatch(SuperComputePipeline *superPipeline, uint32_t nGroupX, uint32_t nGroupY, uint32_t nGroupZ) override;

	virtual void Begin(SuperRenderTarget *renderTarget) override;

	virtual void End() override;

public:
    Swapchain *UpdateSurface(VkExtent2D extent);

    void Prepare(size_t threadCount = 1);

    void UpdateSwapchain(const VkExtent2D &extent, const VkSurfaceTransformFlagBitsKHR transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR);

    void SetupDescriptorSetLayout();

    VkExtent2D GetViewportSize() const
    {
        return surfaceExtent;
    }

    template <ExposedType T>
    inline constexpr T Get()
    {
        if constexpr (IsPrimitiveOf<Instance, T>())
        {
            return instance;
        }
        if constexpr (IsPrimitiveOf<PhysicalDevice, T>())
        {
            return device->Get<PhysicalDevice &>();
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
    }

    template <ExposedType T>
    T *GetAddress()
    {
        if constexpr (IsPrimitiveOf<Device, T>())
        {
            return device;
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

    RenderTarget *GetRenderTarget()
    {
		return present.renderTargets[swapchainIndex];
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

protected:
	void __InitSyncObjects();

    void __SubmitFrame();

    void __PushConstant(Pipeline *pipeline, Shader::Stage stage, uint32_t size, const void *data, uint32_t offset);

private:
    Window *window{ nullptr };

    Instance instance;

    URef<Device> device;

    std::array<Swapchain, 1> swapchainPool;

    Swapchain *swapchain = nullptr;

    Queue *queue{ nullptr };

    struct
    {
	    std::vector<URef<RenderTarget>> renderTargets;
    } present;

    uint32_t swapchainIndex = 0;

    Surface surface;

    VkExtent2D surfaceExtent{ 0, 0 };

    Ref<RenderPass> renderPass;

    struct
    {
        VkSurfaceTransformFlagBitsKHR preTransform{ VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR };
    } regisry;

    size_t threadCount{ 1 };

	URef<SemaphorePool> semaphorePool;

	VkSemaphore timelineSemaphore{VK_NULL_HANDLE};

	uint64_t syncValues[3] = {0, 0, 0};

	uint64_t lastSync = 0;

	std::array<RenderSemaphore, 3> semaphores;

	std::array<VkFence, 3> fences{VK_NULL_HANDLE};

	uint32_t sync{0};
};

}
}
