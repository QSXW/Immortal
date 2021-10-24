#pragma once

#include "Render/Renderer.h"

#include "Common.h"
#include "RenderContext.h"
#include "Texture.h"

namespace Immortal
{
namespace Vulkan
{

class Renderer : public SuperRenderer
{
public:
    using Super = SuperRenderer;

public:
    Renderer(RenderContext::Super *context);

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

    virtual std::shared_ptr<Shader> CreateShader(const std::string &filepath, Shader::Type type) override
    {
        return context->CreateShader(filepath, type);
    }

    virtual std::shared_ptr<SuperTexture> CreateTexture(const std::string &filepath) override
    {
        return std::make_shared<Texture>(context, filepath);
    }

private:
    void Resize();

private:
    RenderContext *context{ nullptr };

    Device *device{ nullptr };

    std::vector<CommandBuffer*> *commandBuffers{ nullptr };

    Swapchain *swapchain{ nullptr };

    std::vector<Semaphores> semaphores;
    
    uint32_t sync{ 0 };

    uint32_t frameSize{ 0 };

    std::vector<VkFence> fences;

    VkRenderPass renderPass{ VK_NULL_HANDLE };

    VkFormat depthFormat{ VK_FORMAT_UNDEFINED };

    Queue *queue{ nullptr };

    uint32_t currentBuffer{ 0 };

    std::unique_ptr<SemaphorePool> semaphorePool;

    std::vector<VkFramebuffer> framebuffers;

    VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };

    VkPipelineStageFlags submitPipelineStages{ VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
};

}
}
