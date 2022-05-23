#include "Pipeline.h"
#include "Device.h"
#include "RenderTarget.h"
#include "Texture.h"
#include "RenderContext.h"

namespace Immortal
{
namespace Vulkan
{

struct TopologyConverter
{
    TopologyConverter(GraphicsPipeline::PrimitiveType type) :
        Type{ type }
    {

    }

    operator VkPrimitiveTopology()
    {
        switch (Type)
        {

        case GraphicsPipeline::PrimitiveType::Point:
            return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        case GraphicsPipeline::PrimitiveType::Line:
            return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        case GraphicsPipeline::PrimitiveType::Triangles:
        default:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        }
    }

    GraphicsPipeline::PrimitiveType Type;
};

Pipeline::Pipeline(Device *device, Shader *shader) :
    device{ device }
{
    layout = shader->Get<PipelineLayout&>();
    descriptor.setLayout  = shader->Get<VkDescriptorSetLayout>();
    descriptor.setUpdater = shader->GetAddress<DescriptorSetUpdater>();
    descriptor.pool.reset(new DescriptorPool{ device, shader->PoolSize() });
}

Pipeline::~Pipeline()
{
    Destroy();
}

void Pipeline::Destroy()
{
    if (device && handle)
    {
        device->DestroyAsync(handle);
    }
}

Anonymous Pipeline::AllocateDescriptorSet(uint64_t uuid)
{
    auto it = descriptor.packs.find(uuid);
    if (it != descriptor.packs.end())
    {
        DescriptorSetPack &pack = it->second;
        descriptor.set = pack.DescriptorSets[pack.Sync];
        SLROTATE(pack.Sync, DescriptorSetPack::Size);
    }
    else
    {
        DescriptorSetPack pack{};
        if (!descriptor.freePacks.empty())
        {
            pack = descriptor.freePacks.front();
            descriptor.freePacks.pop();
        }
        else
        {
            std::array<VkDescriptorSetLayout, DescriptorSetPack::Size> descriptorSetLayouts{};
            for (auto &d : descriptorSetLayouts)
            {
                d = descriptor.setLayout;
            }
            Check(descriptor.pool->Allocate(descriptorSetLayouts.data(), pack.DescriptorSets, DescriptorSetPack::Size));
        }
        descriptor.set = pack.DescriptorSets[pack.Sync];
        SLROTATE(pack.Sync, DescriptorSetPack::Size);

        descriptor.packs[uuid] = pack;
    }
    descriptor.setUpdater->Set(descriptor.set);
    descriptor.setUpdater->Clear();

    return Anonymize(descriptor.set);
}

void Pipeline::FreeDescriptorSet(uint64_t uuid)
{
    auto it = descriptor.packs.find(uuid);
    if (it != descriptor.packs.end())
    {
        DescriptorSetPack &pack = it->second;
        descriptor.freePacks.push(std::move(pack));
        descriptor.packs.erase(it);
    }
}

bool Pipeline::Ready()
{
    bool ready = descriptor.setUpdater->Ready();

    if (ready && !descriptor.set)
    {
        Check(descriptor.pool->Allocate(&descriptor.setLayout, &descriptor.set));
        descriptor.setUpdater->Set(descriptor.set);
    }
    return ready;
}

void Pipeline::Update()
{
    if (Ready())
    {
        descriptor.setUpdater->Update(*device);
    }
}

void Pipeline::Bind(Texture::Super *super, uint32_t slot)
{
    auto texture = dcast<Texture *>(super);
    Bind((const Descriptor *)(&texture->DescriptorInfo()), slot);
}

void Pipeline::Bind(Buffer::Super *super, uint32_t slot)
{
    auto buffer = dcast<Buffer *>(super);
    Bind((const Descriptor *)(buffer->Descriptor()), slot);
}

void Pipeline::Bind(const std::string &name, const Buffer::Super *uniform)
{
    auto bufferDescriptor = rcast<const BufferDescriptor *>(uniform->Descriptor());
    descriptor.setUpdater->Set(name, bufferDescriptor);
    Update();
}

void Pipeline::Bind(const Descriptor *descriptors, uint32_t slot)
{
    auto &writeDescriptor = descriptor.setUpdater->WriteDescriptorSets[slot];
    if (writeDescriptor.descriptorType <= VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)
    {
        writeDescriptor.pImageInfo = rcast<const VkDescriptorImageInfo *>(descriptors);
    }
    else
    {
        writeDescriptor.pBufferInfo = rcast<const VkDescriptorBufferInfo *>(descriptors);
    }
    writeDescriptor.dstSet = descriptor.set;
    Update();
}

GraphicsPipeline::GraphicsPipeline(Device *device, std::shared_ptr<Shader::Super> &shader) :
    Super{ shader },
    NativeSuper{ device, std::dynamic_pointer_cast<Shader>(shader).get() }
{
    state = std::make_unique<State>();
    CleanUpObject(state.get(), 0, offsetof(State, inputAttributeDescriptions));
}

GraphicsPipeline::~GraphicsPipeline()
{

}

void GraphicsPipeline::SetupVertex()
{
    state->inputAssembly.sType    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    state->inputAssembly.flags    = 0;
    state->inputAssembly.topology = TopologyConverter{ desc.primitiveType };
    state->inputAssembly.primitiveRestartEnable = VK_FALSE;
}

void GraphicsPipeline::SetupLayout()
{
    auto &inputAttributeDescriptions = state->inputAttributeDescriptions;
    auto &vertexInputBidings = state->vertexInputBidings;

    state->vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    state->vertexInput.vertexBindingDescriptionCount   = U32(vertexInputBidings.size());
    state->vertexInput.pVertexBindingDescriptions      = vertexInputBidings.data();
    state->vertexInput.vertexAttributeDescriptionCount = U32(inputAttributeDescriptions.size());
    state->vertexInput.pVertexAttributeDescriptions    = inputAttributeDescriptions.data();
}

void GraphicsPipeline::Set(const InputElementDescription &description)
{
    Super::Set(description);
    auto size                       = desc.layout.Size();
    auto &inputAttributeDescription = state->inputAttributeDescriptions;

    inputAttributeDescription.resize(size);
    for (int i = 0; i < size; i++)
    {
        inputAttributeDescription[i].binding  = 0;
        inputAttributeDescription[i].location = i;
        inputAttributeDescription[i].format   = desc.layout[i].BaseType<VkFormat>();
        inputAttributeDescription[i].offset   = desc.layout[i].Offset();
    }

    state->vertexInputBidings.emplace_back(VkVertexInputBindingDescription{
        0,
        desc.layout.Stride,
        VK_VERTEX_INPUT_RATE_VERTEX
        });

    SetupVertex();
    SetupLayout();
}

void GraphicsPipeline::Create(const SuperRenderTarget *superTarget)
{
    Reconstruct(superTarget);
    NativeSuper::Update();
}

void GraphicsPipeline::Reconstruct(const SuperRenderTarget *superTarget)
{
    Destroy();

    auto target = dynamic_cast<const RenderTarget*>(superTarget);

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
    for (size_t i = 0; i < colorBlends.size(); i++)
    {
        auto &colorBlend = colorBlends[i];
        colorBlend.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;;

        if (!i && flags & Pipeline::State::Blend)
        {
            colorBlend.blendEnable         = VK_TRUE;
            colorBlend.colorBlendOp        = VK_BLEND_OP_ADD;
            colorBlend.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
            colorBlend.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            colorBlend.alphaBlendOp        = VK_BLEND_OP_ADD;
            colorBlend.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
            colorBlend.dstAlphaBlendFactor = VK_BLEND_FACTOR_DST_ALPHA;
        }
        else
        {
            colorBlend.blendEnable = VK_FALSE;
        }
    }
    state->colorBlend.sType               = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    state->colorBlend.attachmentCount     = U32(colorBlends.size());
    state->colorBlend.pAttachments        = colorBlends.data();
    state->colorBlend.blendConstants[0]   = 1.0f;
    state->colorBlend.blendConstants[1]   = 1.0f;
    state->colorBlend.blendConstants[2]   = 1.0f;
    state->colorBlend.blendConstants[3]   = 1.0f;

    state->depthStencil.sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    state->depthStencil.depthTestEnable       = flags & Pipeline::State::Depth ? VK_TRUE : VK_FALSE;
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
    state->dynamic.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    state->dynamic.dynamicStateCount = dynamic.size();
    state->dynamic.pDynamicStates = dynamic.data();

    auto shader = std::dynamic_pointer_cast<Shader>(desc.shader);

    VkGraphicsPipelineCreateInfo createInfo{};
    createInfo.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    createInfo.pNext               = nullptr;
    createInfo.renderPass          = target->GetRenderPass();
    createInfo.flags               = 0;
    createInfo.layout              = layout;
    createInfo.pInputAssemblyState = &state->inputAssembly;
    createInfo.pVertexInputState   = &state->vertexInput;
    createInfo.pRasterizationState = &state->rasterization;
    createInfo.pDepthStencilState  = &state->depthStencil;
    createInfo.pViewportState      = &state->viewport;
    createInfo.pMultisampleState   = &state->multiSample;
    createInfo.pDynamicState       = &state->dynamic;
    createInfo.pColorBlendState    = &state->colorBlend;

    auto &stages = shader->GetStages();
    createInfo.pStages    = stages.data();
    createInfo.stageCount = stages.size();

    Check(device->CreatePipelines(cache, 1, &createInfo, &handle));
}

void GraphicsPipeline::CopyState(Super &super)
{
    auto &other = dcast<GraphicsPipeline &>(super);
    memcpy(state.get(), other.state.get(), offsetof(State, inputAttributeDescriptions));

    state->vertexInputBidings         = other.state->vertexInputBidings;
    state->inputAttributeDescriptions = other.state->inputAttributeDescriptions;

    SetupVertex();
    SetupLayout();
}

ComputePipeline::ComputePipeline(Device *device, Shader::Super *superShader) :
    NativeSuper{ device, dcast<Shader *>(superShader) }
{
    auto shader = dynamic_cast<Shader *>(superShader);
    auto &stage = shader->GetStages();

    THROWIF(!shader->IsType(Shader::Type::Compute), "Init compute pipeline with incorrect shader source type")

    VkComputePipelineCreateInfo createInfo{};
    createInfo.sType  = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    createInfo.pNext  = nullptr;
    createInfo.layout = layout;
    createInfo.stage  = stage[0];

    device->CreatePipelines(cache, 1, &createInfo, &handle);
    Update();
}

ComputePipeline::~ComputePipeline()
{

}

void ComputePipeline::Update()
{
    if (Ready())
    {
        descriptor.setUpdater->Update(*device);
    }
}

void ComputePipeline::Bind(Texture::Super *superTexture, uint32_t slot)
{
    auto texture = dynamic_cast<Texture *>(superTexture);
    VkDescriptorImageInfo *descriptorInfo = (VkDescriptorImageInfo *)(&texture->DescriptorInfo());

    auto &writeDescriptor = descriptor.setUpdater->WriteDescriptorSets[slot];
    if (writeDescriptor.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)
    {
        VkImageSubresourceRange subresourceRange{};
        subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        subresourceRange.baseMipLevel   = 0;
        subresourceRange.baseArrayLayer = 0;
        subresourceRange.levelCount     = texture->MipLevels();
        subresourceRange.layerCount     = texture->LayerCount();

        ImageBarrier barrier = {
            *texture,
            subresourceRange,
            texture->Layout,
            VK_IMAGE_LAYOUT_GENERAL,
            0,
            VK_ACCESS_SHADER_WRITE_BIT,
            //device->QueueFailyIndex(Queue::Type::Graphics),
            //device->QueueFailyIndex(Queue::Type::Compute)
        };

        barriers.emplace_back(std::move(barrier));
        descriptorInfo->imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    }
    writeDescriptor.pImageInfo = descriptorInfo;

    Update();
}

void ComputePipeline::Dispatch(CommandBuffer *cmdbuf, uint32_t nGroupX, uint32_t nGroupY, uint32_t nGroupZ)
{
    cmdbuf->PipelineImageBarrier(
        VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        barriers.data(),
        U32(barriers.size())
    );

    cmdbuf->BindPipeline(handle, BindPoint);

    cmdbuf->BindDescriptorSets(
        BindPoint,
        layout,
        0, 1,
        &descriptor.set,
        0, nullptr
    );

    cmdbuf->Dispatch(nGroupX, nGroupY, nGroupZ);

    for (auto &b : barriers)
    {
        b.Swap();
    }

    cmdbuf->PipelineImageBarrier(
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
        barriers.data(),
        U32(barriers.size())
    );

    barriers.clear();
}

void ComputePipeline::Dispatch(uint32_t nGroupX, uint32_t nGroupY, uint32_t nGroupZ)
{
    device->ComputeAsync([&](CommandBuffer *cmdbuf) {
        Dispatch(cmdbuf, nGroupX, nGroupY, nGroupZ);
        });
}

void ComputePipeline::PushConstant(uint32_t size, const void *data, uint32_t offset)
{
    device->ComputeAsync([&](CommandBuffer *cmdbuf) {
        cmdbuf->PushConstants(
            layout,
            Shader::Stage::Compute,
            offset,
            size,
            data
        );
        });
}

}
}
