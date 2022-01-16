#pragma once

#include <array>

#include "Render/Renderer.h"
#include "Common.h"
#include "RenderContext.h"
#include "Texture.h"
#include "Shader.h"
#include "Pipeline.h"
#include "Buffer.h"
#include "Framebuffer.h"
#include "SemaphorePool.h"

namespace Immortal
{
namespace Vulkan
{

class Renderer : public SuperRenderer
{
public:
    using Super = SuperRenderer;

    static inline VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };

public:
    Renderer(RenderContext::Super *context);

    virtual ~Renderer();

    virtual void Setup() override;

    virtual void SwapBuffers() override;

    void SubmitFrame();

    virtual void PrepareFrame() override;

    virtual uint32_t Index() override
    {
        return currentBuffer;
    }

    virtual void OnResize(UINT32 x, UINT32 y, UINT32 width, UINT32 height) override
    {
        Resize();
    }

    virtual const char *GraphicsRenderer()
    {
        return context->GraphicsRenderer();
    }

    virtual Shader::Super *CreateShader(const std::string &filepath, Shader::Type type) override
    {
        return new Shader{ device, filepath, type };
    }

    virtual GraphicsPipeline::Super *CreateGraphicsPipeline(std::shared_ptr<Shader::Super> &shader)
    {
        return new GraphicsPipeline{ device, shader };
    }

    virtual ComputePipeline::Super *CreateComputePipeline(Shader::Super *shader)
    {
        return new ComputePipeline{ device, shader };
    }

    virtual Texture::Super *CreateTexture(const std::string &filepath) override
    {
        return new Texture{ device, filepath };
    }

    virtual Texture::Super *CreateTexture(uint32_t width, uint32_t height, const void *data, const Texture::Description &description) override
    {
        return new Texture{ device, width, height, data, description };
    }

    virtual Buffer::Super *CreateBuffer(const size_t size, const void *data, Buffer::Type type) override
    {
        return new Buffer{ device, size, data, type };
    }

    virtual Buffer::Super *CreateBuffer(const size_t size, Buffer::Type type) override
    {
        return new Buffer{ device, size, type };
    }

    virtual Buffer::Super *CreateBuffer(const size_t size, uint32_t binding) override
    {
        return new Buffer{ device, size, binding };
    }

    virtual RenderTarget::Super *CreateRenderTarget(const RenderTarget::Description &description) override
    {
        return new RenderTarget{ device, description };
    }

    virtual Descriptor *CreateImageDescriptor(uint32_t count) override;

    virtual Descriptor *CreateBufferDescriptor(uint32_t count) override;

    virtual void PushConstant(GraphicsPipeline::Super *pipeline, Shader::Stage stage, uint32_t size, const void *data, uint32_t offset) override;

    virtual void PushConstant(ComputePipeline::Super *pipeline, Shader::Stage stage, uint32_t size, const void *data, uint32_t offset);

    virtual void Draw(GraphicsPipeline::Super *pipeline) override;

    virtual void Dispatch(ComputePipeline::Super *superPipeline, uint32_t nGroupX, uint32_t nGroupY, uint32_t nGroupZ) override;

    virtual void Begin(std::shared_ptr<RenderTarget::Super> &renderTarget) override;

    virtual void End() override;

private:
    void Resize();

private:
    RenderContext *context{ nullptr };

    Device *device{ nullptr };

    Swapchain *swapchain{ nullptr };

    Queue *queue{ nullptr };

    SemaphorePool semaphorePool;

    std::array<Semaphores, 3> semaphores;
    
    std::array<VkFence, 3> fences{ VK_NULL_HANDLE };

    uint32_t sync{ 0 };

    uint32_t currentBuffer{ 0 };
};

}
}
