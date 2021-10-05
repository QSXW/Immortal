#pragma once

#include "Render/Renderer.h"

#include "Common.h"
#include "RenderContext.h"

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

    void Resize();

    virtual const char *GraphicsRenderer()
    {
        return context->GraphicsRenderer();
    }

public:
    RenderContext *context{ nullptr };

    Device *device{ nullptr };

    std::vector<CommandBuffer*> commandBuffers;

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
