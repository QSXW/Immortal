#include "Pipeline.h"
#include "Device.h"
#include "RenderTarget.h"
#include "Texture.h"

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
        SetupVertex();
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

    SetupLayout();
}

void Pipeline::Create(const std::shared_ptr<RenderTarget::Super> &superTarget)
{
    Reconstruct(superTarget);

    auto shader = std::dynamic_pointer_cast<Shader>(desc.shader);
    descriptorSetUpdater = shader->GetAddress<DescriptorSetUpdater>();

    if (Ready())
    {
        descriptorSetUpdater->Update(*device);
    }
}

void Pipeline::Reconstruct(const std::shared_ptr<SuperRenderTarget> &superTarget)
{
    auto target = std::dynamic_pointer_cast<RenderTarget>(superTarget);

    auto state = &configuration->state;
    auto attachment = &configuration->attament;

    state->rasterization.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    state->rasterization.polygonMode             = VK_POLYGON_MODE_FILL;
    state->rasterization.cullMode                = VK_CULL_MODE_NONE;
    state->rasterization.frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    state->rasterization.depthClampEnable        = VK_FALSE;
    state->rasterization.rasterizerDiscardEnable = VK_FALSE;
    state->rasterization.depthBiasEnable         = VK_FALSE;
    state->rasterization.lineWidth               = 1.0f;

    std::vector<VkPipelineColorBlendAttachmentState> colorBlends;
    colorBlends.resize(target->ColorAttachmentCount());
    for (auto &colorBlend : colorBlends)
    {
        colorBlend.colorWriteMask = 0xf;
        colorBlend.blendEnable    = VK_FALSE;
    }
    state->colorBlend.sType               = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    state->colorBlend.attachmentCount     = U32(colorBlends.size());
    state->colorBlend.pAttachments        = colorBlends.data();
    state->colorBlend.blendConstants[0]   = 1.0f;
    state->colorBlend.blendConstants[1]   = 1.0f;
    state->colorBlend.blendConstants[2]   = 1.0f;
    state->colorBlend.blendConstants[3]   = 1.0f;

    state->depthStencil.sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    state->depthStencil.depthTestEnable       = VK_TRUE;
    state->depthStencil.depthWriteEnable      = VK_TRUE;
    state->depthStencil.depthCompareOp        = VK_COMPARE_OP_LESS_OR_EQUAL;
    state->depthStencil.depthBoundsTestEnable = VK_FALSE;
    state->depthStencil.back.failOp           = VK_STENCIL_OP_KEEP;
    state->depthStencil.back.passOp           = VK_STENCIL_OP_KEEP;
    state->depthStencil.back.compareOp        = VK_COMPARE_OP_ALWAYS;
    state->depthStencil.stencilTestEnable     = VK_FALSE;
    state->depthStencil.front                 = state->depthStencil.back;

    state->viewport.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    state->viewport.viewportCount = 1;
    state->viewport.scissorCount  = 1;
    state->viewport.flags         = 0;

    state->multiSample.sType                = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    state->multiSample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    state->multiSample.flags                = 0;

    std::array<VkDynamicState, 2> dynamic{
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
    };
    state->dynamic.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    state->dynamic.dynamicStateCount = dynamic.size();
    state->dynamic.pDynamicStates    = dynamic.data();

    if (!desc.shader->IsGraphics())
    {
        bindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;
    }

    auto shader = std::dynamic_pointer_cast<Shader>(desc.shader);
    descriptorSet = shader->Get<VkDescriptorSet>();

    VkGraphicsPipelineCreateInfo createInfo{};
    createInfo.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    createInfo.pNext               = nullptr;
    createInfo.renderPass          = target->GetRenderPass();
    createInfo.flags               = 0;
    createInfo.layout              = shader->Get<PipelineLayout&>();
    createInfo.pInputAssemblyState = &state->inputAssembly;
    createInfo.pVertexInputState   = &state->vertexInput;
    createInfo.pRasterizationState = &state->rasterization;
    createInfo.pDepthStencilState  = &state->depthStencil;
    createInfo.pViewportState      = &state->viewport;
    createInfo.pMultisampleState   = &state->multiSample;
    createInfo.pDynamicState       = &state->dynamic;
    createInfo.pColorBlendState    = &state->colorBlend;

    auto &stages = shader->Stages();
    createInfo.pStages    = stages.data();
    createInfo.stageCount = stages.size();
  
    Check(device->CreatePipelines(cache, 1, &createInfo, nullptr, &handle));
}

void Pipeline::Bind(const std::shared_ptr<SuperTexture> &superTexture, uint32_t slot)
{
    for (auto &writeDescriptor : descriptorSetUpdater->WriteDescriptorSets)
    {
        if (writeDescriptor.descriptorType <= VK_DESCRIPTOR_TYPE_STORAGE_IMAGE &&
            writeDescriptor.dstBinding == slot)
        {
            auto texture = std::dynamic_pointer_cast<Texture>(superTexture);
            writeDescriptor.pImageInfo = texture->DescriptorInfo();
            break;
        }
    }
    if (Ready())
    {
        descriptorSetUpdater->Update(*device);
    }
}

void Pipeline::Bind(const std::string &name, const Buffer::Super *uniform)
{
    descriptorSetUpdater->Set(name, Deanonymize<BufferDescriptor>(uniform->Descriptor()));
    if (Ready())
    {
        descriptorSetUpdater->Update(*device);
    }
}

}
}
