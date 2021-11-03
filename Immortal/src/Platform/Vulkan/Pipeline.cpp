#include "Pipeline.h"
#include "Device.h"

namespace Immortal
{
namespace Vulkan
{

Pipeline::Pipeline(Device *device, std::shared_ptr<Shader::Super> &shader) :
    device{ device },
    Super{ shader }
{
    configuration = std::make_unique<Configuration>();
    CleanUpObject(configuration.get());
}

Pipeline::~Pipeline()
{

}

void Pipeline::Set(std::shared_ptr<Buffer::Super> &buffer)
{
    if (buffer->GetType() == Buffer::Type::Vertex)
    {
        desc.vertexBuffers.emplace_back(buffer);
        INITVertex();
    }
    if (buffer->GetType() == Buffer::Type::Index)
    {
        desc.indexBuffer = buffer;
    }
}

void Pipeline::Set(const InputElementDescription &description)
{
    Super::Set(description);
    auto size                       = desc.layout.Size();
    auto &inputAttributeDescription = configuration->inputAttributeDescriptions;

    inputAttributeDescription.resize(size);
    for (int i = 0; i < size; i++)
    {
        inputAttributeDescription[i].binding  = 0;
        inputAttributeDescription[i].location = i;
        inputAttributeDescription[i].format   = desc.layout[i].BaseType<VkFormat>();
        inputAttributeDescription[i].offset   = desc.layout[i].Offset();
    }

    configuration->vertexInputBidings.emplace_back(VkVertexInputBindingDescription{
        0,
        desc.layout.Stride(),
        VK_VERTEX_INPUT_RATE_VERTEX
        });

    INITLayout();
}

void Pipeline::Create(std::shared_ptr<Framebuffer::Super> &superFramebuffer)
{
    framebuffer = std::dynamic_pointer_cast<Framebuffer>(superFramebuffer);

    auto state = &configuration->state;
    auto attachment = &configuration->attament;
    {
        state->rasterization.sType            = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        state->rasterization.polygonMode      = VK_POLYGON_MODE_FILL;
        state->rasterization.cullMode         = VK_CULL_MODE_NONE;
        state->rasterization.frontFace        = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        state->rasterization.flags            = 0;
        state->rasterization.depthClampEnable = VK_FALSE;
        state->rasterization.lineWidth        = 1.0f;
    }

    {
        attachment->colorBlend.colorWriteMask = 0xf;
	    attachment->colorBlend.blendEnable    = VK_TRUE;
        state->colorBlend.sType               = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        state->colorBlend.attachmentCount     = 1;
        state->colorBlend.pAttachments        = &attachment->colorBlend;
        state->colorBlend.blendConstants[0]   = 1.0f;
        state->colorBlend.blendConstants[1]   = 1.0f;
        state->colorBlend.blendConstants[2]   = 1.0f;
        state->colorBlend.blendConstants[3]   = 1.0f;

    }

    {
        state->depthStencil.sType            = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        state->depthStencil.depthTestEnable  = VK_TRUE;
        state->depthStencil.depthWriteEnable = VK_TRUE;
        state->depthStencil.depthCompareOp   = VK_COMPARE_OP_GREATER;
        state->depthStencil.front            =  state->depthStencil.back;
        state->depthStencil.back.compareOp   = VK_COMPARE_OP_ALWAYS;
    }

    {
        state->viewport.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        state->viewport.viewportCount = 1;
        state->viewport.scissorCount  = 1;
        state->viewport.flags         = 0;
    }

    {
        state->multiSample.sType                = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        state->multiSample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        state->multiSample.flags                = 0;
    }

    {
        state->dynamic.dynamicStateCount = configuration->dynamic.size();
        state->dynamic.pDynamicStates    = configuration->dynamic.data();
    }

    if (!desc.shader->IsGraphics())
    {
        bindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;
    }

    VkGraphicsPipelineCreateInfo createInfo{};
    createInfo.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    createInfo.renderPass          = framebuffer->Get<RenderPass>();
    createInfo.flags               = 0;
    createInfo.pInputAssemblyState = &state->inputAssembly;
    createInfo.pVertexInputState   = &state->vertexInput;
    createInfo.pRasterizationState = &state->rasterization;
    createInfo.pDepthStencilState  = &state->depthStencil;
    createInfo.pViewportState      = &state->viewport;
    createInfo.pMultisampleState   = &state->multiSample;
    createInfo.pDynamicState       = &state->dynamic;

    auto &stages = std::dynamic_pointer_cast<Shader>(desc.shader)->Stages();
    createInfo.pStages    = stages.data();
    createInfo.stageCount = stages.size();
  
    Check(device->CreatePipelines(cache, 1, &createInfo, nullptr, &handle));
}

}
}
