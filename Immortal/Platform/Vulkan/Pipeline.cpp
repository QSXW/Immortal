#include "Pipeline.h"
#include "Device.h"
#include "RenderTarget.h"
#include "Texture.h"

namespace Immortal
{
namespace Vulkan
{

VkPrimitiveTopology CAST(GraphicsPipeline::PrimitiveType type)
{
    switch (type)
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

Pipeline::Pipeline(Device *device) :
    device{ device },
    pipelineCache{ VK_NULL_HANDLE },
    pipelineLayout{},
    descriptorSetLayout{ VK_NULL_HANDLE },
    bindPoint{}
{

}

Pipeline::~Pipeline()
{
    Destroy();
}

void Pipeline::Destroy()
{
    if (device)
    {
	    device->Destroy(descriptorSetLayout);
        device->Destroy((VkPipelineLayout)pipelineLayout);
        device->DestroyAsync(handle);
        handle = VK_NULL_HANDLE;
        device = nullptr;
    }
}

void Pipeline::ConstructPipelineLayout(const std::vector<VkDescriptorSetLayoutBinding> &descriptorSetLayoutBindings, const std::vector<VkPushConstantRange> &pushConstantRanges)
{
    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {
        .sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext        = nullptr,
	    .bindingCount = uint32_t(descriptorSetLayoutBindings.size()),
	    .pBindings    = descriptorSetLayoutBindings.data(),
    };

    Check(device->Create(&descriptorSetLayoutCreateInfo, &descriptorSetLayout));

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {
        .sType                   = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext                   = nullptr,
        .flags                   = 0,
        .setLayoutCount          = 1,
        .pSetLayouts             = &descriptorSetLayout,
        .pushConstantRangeCount  = uint32_t(pushConstantRanges.size()),
        .pPushConstantRanges     = pushConstantRanges.data()
    };

    PipelineLayout{
	    *device,
	    &pipelineLayoutCreateInfo
    }.Swap(pipelineLayout);
}

GraphicsPipeline::GraphicsPipeline(Device *device) :
    Pipeline{ device }
{
	SetBindPoint(VK_PIPELINE_BIND_POINT_GRAPHICS);
}

GraphicsPipeline::GraphicsPipeline(Device *device, Ref<Shader::Super> shader) :
    Pipeline{ device }
{

}

GraphicsPipeline::GraphicsPipeline(Device *device, Shader *shader) :
    Pipeline{ device }
{

}

GraphicsPipeline::~GraphicsPipeline()
{

}

void GraphicsPipeline::Construct(SuperShader **_ppShader, size_t shaderCount, const InputElementDescription &description, const std::vector<Format> &outputDescription)
{
    std::vector<VkPipelineShaderStageCreateInfo> stageCreateInfo;
    stageCreateInfo.reserve(shaderCount);

    std::vector<VkPushConstantRange> pushConstantRanges;
	std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings;

    Shader **ppShader = (Shader **)(_ppShader);
    for (size_t i = 0; i < shaderCount; i++)
    {
        Shader *shader = ppShader[i];
        stageCreateInfo.emplace_back(VkPipelineShaderStageCreateInfo{
            .sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .pNext               = nullptr,
            .flags               = 0,
            .stage               = shader->GetStage(),
            .module              = *shader,
            .pName               = shader->GetEntryPoint(),
            .pSpecializationInfo = nullptr,
        });

        pushConstantRanges.insert(pushConstantRanges.end(), shader->GetPushConstantRanges().begin(), shader->GetPushConstantRanges().end());
		descriptorSetLayoutBindings.insert(descriptorSetLayoutBindings.end(), shader->GetDescriptorSetLayoutBinding().begin(), shader->GetDescriptorSetLayoutBinding().end());
    }

    ConstructPipelineLayout(descriptorSetLayoutBindings, pushConstantRanges);

    std::vector<VkVertexInputAttributeDescription> vertexInputAttributeDescriptions;
    vertexInputAttributeDescriptions.reserve(description.Size());
	for (uint32_t i = 0; i < description.Size(); i++)
    {
        vertexInputAttributeDescriptions.emplace_back(VkVertexInputAttributeDescription{
		    .location = i,
            .binding  = 0,
            .format   = description[i].GetFormat(),
            .offset   = description[i].GetOffset(),
        });
    }

    VkVertexInputBindingDescription vertexInputBindingDescription = {
        .binding   = 0,
		.stride    = description.GetStride(),
		.inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
    };

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo = {
		.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .pNext                  = nullptr,
        .flags                  = 0,
		.topology               = CAST(PrimitiveType::Triangles),
        .primitiveRestartEnable = VK_FALSE,
    };

    VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo = {
		.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .pNext                           = nullptr,
        .flags                           = 0,
        .vertexBindingDescriptionCount   = 1,
        .pVertexBindingDescriptions      = &vertexInputBindingDescription,
		.vertexAttributeDescriptionCount = uint32_t(vertexInputAttributeDescriptions.size()),
		.pVertexAttributeDescriptions = vertexInputAttributeDescriptions.data()
    };

    VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo = {
	    .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .pNext                   = nullptr, 
        .flags                   = 0,
        .depthClampEnable        = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode             = VK_POLYGON_MODE_FILL,
        .cullMode                = VK_CULL_MODE_NONE,
        .frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .depthBiasEnable         = VK_FALSE,
        .depthBiasConstantFactor = 0,
        .depthBiasClamp          = 0,
        .depthBiasSlopeFactor    = 0,
        .lineWidth               = 1.0f,
    };

    std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachmentStates;
	colorBlendAttachmentStates.resize(outputDescription.size());
	for (size_t i = 0; i < colorBlendAttachmentStates.size(); i++)
    {
		auto &colorBlendAttachmentState = colorBlendAttachmentStates[i];

		colorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        if (!i && flags & Pipeline::State::Blend)
        {
            colorBlendAttachmentState.blendEnable         = VK_TRUE;
            colorBlendAttachmentState.colorBlendOp        = VK_BLEND_OP_ADD;
            colorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
            colorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            colorBlendAttachmentState.alphaBlendOp        = VK_BLEND_OP_ADD;
            colorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            colorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        }
        else
        {
			colorBlendAttachmentState.blendEnable = VK_FALSE;
        }
    }

    VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo = {
        .sType              = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .pNext              = nullptr,
        .flags              = 0,
        .logicOpEnable      = {},
        .logicOp            = {},
	    .attachmentCount    = uint32_t(colorBlendAttachmentStates.size()),
        .pAttachments       = colorBlendAttachmentStates.data(), 
	    .blendConstants     = { 1.0f, 1.0f, 1.0f, 1.0f }
	};

    VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo = {
        .sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .pNext                 = nullptr,
	    .flags                 = {},
        .depthTestEnable       = flags & Pipeline::State::Depth ? VK_TRUE : VK_FALSE,
        .depthWriteEnable      = VK_TRUE,
        .depthCompareOp        = VK_COMPARE_OP_LESS_OR_EQUAL,
        .depthBoundsTestEnable = VK_FALSE,
        .stencilTestEnable     = VK_FALSE,
        .front                 = {
            .failOp      = VK_STENCIL_OP_KEEP,
            .passOp      = VK_STENCIL_OP_KEEP,
	        .depthFailOp = {},
            .compareOp   = VK_COMPARE_OP_ALWAYS,
	        .compareMask = {},
            .writeMask   = {},
            .reference   = {},
        },
        .back            = depthStencilStateCreateInfo.front,
        .minDepthBounds  = 0,
        .maxDepthBounds  = 0,
	};

    VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {
        .sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .pNext         = nullptr,
        .flags         = 0,
        .viewportCount = 1,
        .pViewports    = {},
        .scissorCount  = 1,
	    .pScissors     = {},
	};

    VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo = {
		.sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .pNext                 = nullptr,
        .flags                 = 0,
	    .rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable   = {},
        .minSampleShading      = {},
        .pSampleMask           = {},
	    .alphaToCoverageEnable = {},
        .alphaToOneEnable      = {},
	};

    const VkDynamicState dynamicState[] = {
	    VK_DYNAMIC_STATE_VIEWPORT,
	    VK_DYNAMIC_STATE_SCISSOR,
	    VK_DYNAMIC_STATE_BLEND_CONSTANTS,
    };

    VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {
        .sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .pNext             = nullptr,
        .flags             = 0,
		.dynamicStateCount = SL_ARRAY_LENGTH(dynamicState),
        .pDynamicStates    = dynamicState,
    };

    VkFormat depthAttachmentFormat   = VK_FORMAT_UNDEFINED;
	VkFormat stencilAttachmentFormat = VK_FORMAT_UNDEFINED;
    LightArray<VkFormat> colorAttachmentFormats;
    for (auto &format : outputDescription)
    {
        if (format.IsDepth())
        {
			depthAttachmentFormat = format;
        }
        else
        {
			colorAttachmentFormats.emplace_back(format);
        }
    }

    VkPipelineRenderingCreateInfoKHR renderingCreateInfoKHR {
        .sType                   = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR,
        .pNext                   = nullptr,
        .viewMask                = {},
        .colorAttachmentCount    = uint32_t(colorAttachmentFormats.size()),
	    .pColorAttachmentFormats = colorAttachmentFormats.data(),
        .depthAttachmentFormat   = depthAttachmentFormat,
        .stencilAttachmentFormat = stencilAttachmentFormat,
    };

    VkGraphicsPipelineCreateInfo createInfo = {
        .sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext               = &renderingCreateInfoKHR,
        .flags               = 0,
	    .stageCount          = uint32_t(stageCreateInfo.size()),
        .pStages             = stageCreateInfo.data(),
        .pVertexInputState   = &vertexInputStateCreateInfo,
        .pInputAssemblyState = &inputAssemblyStateCreateInfo,
        .pTessellationState  = nullptr,
        .pViewportState      = &viewportStateCreateInfo,
        .pRasterizationState = &rasterizationStateCreateInfo,
        .pMultisampleState   = &multisampleStateCreateInfo,
        .pDepthStencilState  = &depthStencilStateCreateInfo,
        .pColorBlendState    = &colorBlendStateCreateInfo,
        .pDynamicState       = &dynamicStateCreateInfo,
        .layout              = pipelineLayout,
        .renderPass          = VK_NULL_HANDLE,
        .subpass             = 0,
        .basePipelineHandle  = VK_NULL_HANDLE,
        .basePipelineIndex   = 0,
    };

    Check(device->CreatePipelines(pipelineCache, 1, &createInfo, &handle));
}

ComputePipeline::ComputePipeline(Device *device, Shader::Super *superShader) :
    Pipeline{ device }
{
    Shader *shader = InterpretAs<Shader>(superShader);

    THROWIF(shader->GetStage() == VK_SHADER_STAGE_COMPUTE_BIT, "Init compute pipeline with incorrect shader source type");

    VkPipelineShaderStageCreateInfo shaderStageCreateInfo = {
        .sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext               = nullptr,
        .flags               = 0,
        .stage               = shader->GetStage(),
        .module              = *shader,
        .pName               = shader->GetEntryPoint(),
        .pSpecializationInfo = nullptr,
    };

    VkComputePipelineCreateInfo createInfo{
        .sType              = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
        .pNext              = nullptr,
        .flags              = {},
        .stage              = shaderStageCreateInfo,
        .layout             = pipelineLayout,
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex  = 0,
    };

    device->CreatePipelines(pipelineCache, 1, &createInfo, &handle);
}

ComputePipeline::~ComputePipeline()
{

}

}
}
