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

    virtual void RenderFrame() override;

    virtual uint32_t Index() override
    {
        return currentBuffer;
    }

public:
    RenderContext *context{ nullptr };

    Device *device{ nullptr };

    CommandBuffer *commandBuffer;

    Swapchain *swapchain{ nullptr };

    std::vector<Semaphores> semaphores;

    uint32_t semaphoresIndex{ 0 };

    VkFence fence{ VK_NULL_HANDLE };

    VkRenderPass renderPass{ VK_NULL_HANDLE };

    VkFormat depthFormat{ VK_FORMAT_UNDEFINED };

    Queue *queue{ nullptr };

    UINT32 currentBuffer{ 0 };

    std::unique_ptr<SemaphorePool> semaphorePool;

    std::vector<VkFramebuffer> framebuffers;

    VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };

    VkPipelineStageFlags submitPipelineStages{ VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
};

}
}
