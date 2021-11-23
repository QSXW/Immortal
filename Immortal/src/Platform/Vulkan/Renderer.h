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

namespace Immortal
{
namespace Vulkan
{

class Renderer : public SuperRenderer
{
public:
    using Super = SuperRenderer;

    static inline VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };

    static inline VkPipelineStageFlags submitPipelineStages{ VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

public:
    Renderer(RenderContext::Super *context);

    virtual ~Renderer();

    virtual void INIT() override;

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

    virtual std::shared_ptr<Shader::Super> CreateShader(const std::string &filepath, Shader::Type type) override
    {
        return std::make_shared<Shader>(device, filepath, type);
    }

    virtual std::shared_ptr<Pipeline::Super> CreatePipeline(std::shared_ptr<Shader::Super> &shader)
    {
        return std::make_shared<Pipeline>(device, shader);
    }

    virtual std::shared_ptr<SuperTexture> CreateTexture(const std::string &filepath) override
    {
        return std::make_shared<Texture>(device, filepath);
    }

    virtual std::shared_ptr<SuperTexture> CreateTexture(uint32_t width, uint32_t height, const void *data, const Texture::Description &description) override
    {
        return std::make_shared<Texture>(device, width, height, data, description);
    }

    virtual std::shared_ptr<SuperBuffer> CreateBuffer(const size_t size, const void *data, Buffer::Type type) override
    {
        return std::make_shared<Buffer>(device, size, data, type);
    }

    virtual std::shared_ptr<SuperBuffer> CreateBuffer(const size_t size, Buffer::Type type) override
    {
        return std::make_shared<Buffer>(device, size, type);
    }

    virtual std::shared_ptr <Framebuffer::Super> CreateFramebuffer(const Framebuffer::Description &description) override
    {
        return std::make_shared<Framebuffer>(device, description);
    }

    virtual void Draw(const std::shared_ptr<Pipeline::Super> &pipeline) override
    {
        
    }

    virtual void Begin(std::shared_ptr<Framebuffer::Super> &renderTarget) override
    {
        auto nativeRenderTarget = std::dynamic_pointer_cast<Framebuffer>(renderTarget);
        static VkClearValue clearValues[] = {
            {{ .40f, 0.45f, 0.60f, 0.0f }},
            {{  .0f,  .0f,    .0f, 0.0f }}
        };
        context->Begin([&](CommandBuffer *cmdbuf) {
            VkRenderPassBeginInfo beginInfo{};
            beginInfo.sType                     = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            beginInfo.pNext                     = nullptr;
            beginInfo.framebuffer               = *nativeRenderTarget;
            beginInfo.clearValueCount           = 2;
            beginInfo.pClearValues              = clearValues;
            beginInfo.renderPass                = nativeRenderTarget->Get<RenderPass>();
            beginInfo.renderArea.extent.width   = nativeRenderTarget->Desc().Width;
            beginInfo.renderArea.extent.height  = nativeRenderTarget->Desc().Height;
            beginInfo.renderArea.offset = { 0, 0 };

            cmdbuf->BeginRenderPass(&beginInfo, VK_SUBPASS_CONTENTS_INLINE);
        });
    }

    virtual void End() override
    {
        context->End([](CommandBuffer *cmdbuf) {
            cmdbuf->EndRenderPass();
            });
    }

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
