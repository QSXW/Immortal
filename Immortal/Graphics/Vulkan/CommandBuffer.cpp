#include "CommandBuffer.h"

#include "CommandPool.h"
#include "Device.h"
#include "Buffer.h"
#include "RenderTarget.h"
#include "Pipeline.h"
#include "DescriptorSet.h"

namespace Immortal
{
namespace Vulkan
{

static VkImageLayout CAST(const ImageLayout &layout)
{
	switch (layout)
	{
		case ImageLayout::Undefined:
			return VK_IMAGE_LAYOUT_UNDEFINED;

		case ImageLayout::General:
			return VK_IMAGE_LAYOUT_GENERAL;

		case ImageLayout::Present:
			return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		case ImageLayout::GenericRead:
			return VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;

		case ImageLayout::RenderTarget:
			return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		case ImageLayout::UnorderedAccess:
			return VK_IMAGE_LAYOUT_GENERAL;

		case ImageLayout::DepthStencilWrite:
			return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		case ImageLayout::DepthStencilRead:
			return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

		case ImageLayout::ShaderResource:
			return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		case ImageLayout::TransferSource:
			return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

		case ImageLayout::TransferDestination:
			return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

		case ImageLayout::ResolveSource:
			return VK_IMAGE_LAYOUT_GENERAL;

		case ImageLayout::ResolveDestination:
			return VK_IMAGE_LAYOUT_GENERAL;

		case ImageLayout::ShadingRateSource:
			return VK_IMAGE_LAYOUT_FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL_KHR;

		case ImageLayout::VideoDecodeRead:
			return VK_IMAGE_LAYOUT_VIDEO_DECODE_SRC_KHR;

		case ImageLayout::VideoDecodeWrite:
			return VK_IMAGE_LAYOUT_VIDEO_DECODE_DST_KHR;

		case ImageLayout::VideoEncodeRead:
			    return VK_IMAGE_LAYOUT_VIDEO_ENCODE_SRC_KHR;

		case ImageLayout::VideoEncodeWrite:
			return VK_IMAGE_LAYOUT_VIDEO_ENCODE_DST_KHR;

		default:
			return VK_IMAGE_LAYOUT_UNDEFINED;
	}
}

static VkPipelineStageFlagBits CAST(PipelineStage stage)
{
	switch (stage)
	{
		case PipelineStage::None:
			return VK_PIPELINE_STAGE_NONE;

		case PipelineStage::All:
			return VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

		case PipelineStage::Draw:
			return VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;

		case PipelineStage::VertexInput:
			return VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;

		case PipelineStage::VertexShading:
			return VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;

		case PipelineStage::PixelShading:
			return VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

		case PipelineStage::DepthStencil:
			return VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;

		case PipelineStage::RenderTarget:
			return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

		case PipelineStage::ComputeShading:
			return VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

		case PipelineStage::Raytracing:
			return VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR;

		case PipelineStage::Transfer:
			return VK_PIPELINE_STAGE_TRANSFER_BIT;

		case PipelineStage::AllShading:
			return VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

		default:
			return VK_PIPELINE_STAGE_NONE;
	}
}

VkShaderStageFlags CAST(ShaderStage stage)
{
	VkShaderStageFlags flags = 0;
	if (stage & ShaderStage::Vertex)
	{
		flags |= VK_SHADER_STAGE_VERTEX_BIT;
	}
	if (stage & ShaderStage::Pixel)
	{
		flags |= VK_SHADER_STAGE_FRAGMENT_BIT;
	}
	if (stage & ShaderStage::Compute)
	{
		flags |= VK_SHADER_STAGE_COMPUTE_BIT;
	}
	if (stage & ShaderStage::Mesh)
	{
		flags |= VK_SHADER_STAGE_MESH_BIT_EXT;
	}
	
	return flags;
}

CommandBuffer::CommandBuffer(CommandPool *commandPool, VkCommandBufferLevel level) :
    Handle{},
    commandPool{ commandPool },
    count{ 0 },
    state{ State::Initial },
    level{ level }
{
    Check(commandPool->Allocate(&handle, 1, level));
}

CommandBuffer::~CommandBuffer()
{
	Destroy(commandPool);
}

void CommandBuffer::Destroy(CommandPool *commandPool)
{
    if (handle != VK_NULL_HANDLE)
    {
        commandPool->Release(&handle, 1);
        handle = VK_NULL_HANDLE;
    }
}

VkResult CommandBuffer::Begin(VkCommandBufferUsageFlags flags, CommandBuffer *primaryCommandBuffer, const VkCommandBufferInheritanceInfo *pInheritanceInfo)
{
    if (level == VK_COMMAND_BUFFER_LEVEL_SECONDARY)
    {
        SLASSERT(primaryCommandBuffer && "A primary command buffer pointer must be provided when calling begin from a second one");
    }

    if (Recording())
    {
        LOG::ERR("Command buffer is already recording, call end first then begin");
        return VK_NOT_READY;
    }
    state = State::Recording;

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags            = flags;
    beginInfo.pInheritanceInfo = pInheritanceInfo;

    return vkBeginCommandBuffer(handle, &beginInfo);
}

void CommandBuffer::End()
{
    state = State::Executable;
    Check(EndCommandBuffer());;
}

bool CommandBuffer::IsActive()
{
	return true;
}

void CommandBuffer::Reset()
{
	count = 0;
	state = State::Initial;

    VkCommandBufferResetFlags flags = VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT;
	Check(ResetCommandBuffer(flags));
}

void CommandBuffer::Begin()
{
	Check(Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT));
}

void CommandBuffer::Close()
{

}

void CommandBuffer::BeginEvent(const char *pData, size_t size)
{
	VkDebugUtilsLabelEXT label = {
		.sType      = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
		.pNext      = nullptr,
		.pLabelName = pData,
		.color      = {}
	};
	BeginDebugUtilsLabelEXT(&label);
}

void CommandBuffer::EndEvent()
{
	EndDebugUtilsLabelEXT();
}

void CommandBuffer::SetPipeline(SuperPipeline *_pipeline)
{
	pipeline = InterpretAs<Pipeline>(_pipeline);
	BindPipeline(pipeline->GetBindPoint(), *pipeline);
}

void CommandBuffer::SetDescriptorSet(SuperDescriptorSet *_descriptorSet)
{
	DescriptorSet *descriptorSet = InterpretAs<DescriptorSet>(_descriptorSet);

    VkDescriptorSet descriptorSets[] = { *descriptorSet };
	BindDescriptorSets(pipeline->GetBindPoint(), pipeline->GetPipelineLayout(), 0, 1, descriptorSets, 0, nullptr);
}

void CommandBuffer::SetVertexBuffers(uint32_t firstSlot, uint32_t bufferCount, SuperBuffer **_ppBuffer, uint32_t /*strideInBytes */)
{
	LightArray<VkBuffer> buffers;
	buffers.resize(bufferCount);

	LightArray<VkDeviceSize> bufferOffsets;
	bufferOffsets.resize(bufferCount);

    Buffer **ppBuffer = (Buffer **)(_ppBuffer);
    for (uint32_t i = 0; i < bufferCount; i++)
    {
		buffers[i] = *ppBuffer[i];
    }

	BindVertexBuffers(firstSlot, bufferCount, buffers.data(), bufferOffsets.data());
}

void CommandBuffer::SetIndexBuffer(SuperBuffer *_buffer, Format format)
{
	Buffer *buffer = InterpretAs<Buffer>(_buffer);

    VkIndexType indexType = VK_INDEX_TYPE_UINT16;
    if (format == Format::UINT32)
    {
		indexType = VK_INDEX_TYPE_UINT32;
    }
	BindIndexBuffer(*buffer, buffer->GetOffset(), indexType);
}

void CommandBuffer::SetScissors(uint32_t count, const Rect2D *pScissor)
{
	LightArray<VkRect2D> rect{};
	rect.resize(count);

    for (uint32_t i = 0; i < count; i++)
    {
		rect[i] = VkRect2D{
			.offset = {
			    .x = (int)pScissor[i].left,
			    .y = (int)pScissor[i].top,
            },
			.extent = {
			    .width  = pScissor[i].right - pScissor[i].left,
			    .height = pScissor[i].bottom - pScissor[i].top
            },
        };
    }

	SetScissor(0, count, rect.data());
}

void CommandBuffer::SetBlendFactor(const float factor[4])
{
	SetBlendConstants(factor);
}

void CommandBuffer::PushConstants(ShaderStage stage, const void *pData, uint32_t size, uint32_t offset)
{
	PushConstants(pipeline->GetPipelineLayout(), CAST(stage), offset, size, pData);
}

void CommandBuffer::BeginRenderTarget(SuperRenderTarget *_renderTarget, const ClearValue *pClearValues)
{
	RenderTarget *renderTarget = InterpretAs<RenderTarget>(_renderTarget);

    auto &colorAttachments = renderTarget->GetColorAttachments();
	auto &depthAttachment  = renderTarget->InternalGetDepthAttachment();
    auto &[width, height, depth] = colorAttachments[0].GetExtent();

    if (true /*dynamic rendering*/)
    {
        LightArray<VkRenderingAttachmentInfo> colorAttachmentInfos;
	    colorAttachmentInfos.resize(colorAttachments.size());

        for (size_t i = 0; i < colorAttachments.size(); i++)
        {
		    colorAttachmentInfos[i] = VkRenderingAttachmentInfo{
                .sType              = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
                .pNext              = nullptr,
			    .imageView          = colorAttachments[i].GetImageView(),
			    .imageLayout        = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .resolveMode        = {},
			    .resolveImageView   = {},
			    .resolveImageLayout = {},
                .loadOp             = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp            = VK_ATTACHMENT_STORE_OP_STORE,
                .clearValue         = {
			        .color = *(const VkClearColorValue *)&pClearValues[i],
                },
            };

            VkImageSubresourceRange subresourceRange{
                .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel   = 0,
			    .levelCount     = colorAttachments[i].GetMipLevels(),
                .baseArrayLayer = 0,
			    .layerCount     = colorAttachments[i].GetArrayLayers(),
            };

            colorImageBarriers.emplace_back(ImageBarrier{
			    colorAttachments[i],
                subresourceRange,
			    colorAttachments[i].GetLayout(),
			    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                VK_ACCESS_NONE,
                VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			});
        };

        VkRenderingAttachmentInfo depthAttachmentInfo{};
        if (depthAttachment)
        {
			auto &clearValue = pClearValues[colorAttachments.size()];
		    depthAttachmentInfo = VkRenderingAttachmentInfo{
                .sType              = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
                .pNext              = nullptr,
			    .imageView          = depthAttachment.GetImageView(),
			    .imageLayout        = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
                .resolveMode        = {},
			    .resolveImageView   = {},
			    .resolveImageLayout = {},
                .loadOp             = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp            = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                .clearValue         = {
					.depthStencil = {
			            .depth   = clearValue.depthStencil.depth,
			            .stencil = clearValue.depthStencil.stencil,
					}
				},
            };

            VkImageSubresourceRange subresourceRange{
                .aspectMask     = VK_IMAGE_ASPECT_DEPTH_BIT,
                .baseMipLevel   = 0,
			    .levelCount     = depthAttachment.GetMipLevels(),
                .baseArrayLayer = 0,
			    .layerCount     = depthAttachment.GetArrayLayers(),
            };

            depthImageBarriers.emplace_back(ImageBarrier{
			    depthAttachment,
			    subresourceRange,
			    depthAttachment.GetLayout(),
			    VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
			    VK_ACCESS_NONE,
			    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			});

            auto &barrier = depthImageBarriers[0];

            bool isDepthOnly = IsDepthOnlyFormat(depthAttachment.GetFormat());
			if (!isDepthOnly)
			{
				barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
				barrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			}

            PipelineImageBarrier(
			    VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
			    VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
			    &barrier,
			    1
            );

            barrier.Swap();
			if (barrier.newLayout == VK_IMAGE_LAYOUT_UNDEFINED)
            {
				barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
				depthAttachment.SetLayout(barrier.newLayout);
            }
        };

        PipelineImageBarrier(
		    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		    colorImageBarriers.data(),
		    uint32_t(colorImageBarriers.size())
        );

        for (size_t i = 0; i < colorImageBarriers.size(); i++)
		{
			auto &barrier = colorImageBarriers[i];
			barrier.Swap();
            if (colorAttachments[i].GetLayout() == VK_IMAGE_LAYOUT_UNDEFINED)
            {
				barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
				colorAttachments[i].SetLayout(barrier.newLayout);
            }
        }
        if (renderTarget->IsSwapchainTarget())
        {
			colorImageBarriers[0].newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			colorAttachments[0].SetLayout(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
        }

	    VkRenderingInfo renderingInfo = {
            .sType                = VK_STRUCTURE_TYPE_RENDERING_INFO,
            .pNext                = nullptr,
	        .flags                = {},
            .renderArea           = {
	            .offset = {},
                .extent = { width, height }
            },
	        .layerCount           = colorAttachments[0].GetArrayLayers(),
	        .viewMask             = {},
	        .colorAttachmentCount = uint32_t(colorAttachmentInfos.size()),
	        .pColorAttachments    = colorAttachmentInfos.data(),
	        .pDepthAttachment     = depthAttachment ? &depthAttachmentInfo : nullptr,
	        .pStencilAttachment   = nullptr,
	    };

	    vkCmdBeginRenderingKHR(handle, &renderingInfo);
    }
    else
    {
		uint32_t clearValueSize = colorAttachments.size() + (depthAttachment ? 1 : 0);

        VkRenderPassBeginInfo beginInfo = {
             .sType           = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
             .pNext           = nullptr,
             .renderPass      = renderTarget->GetRenderPass(),
             .framebuffer     = renderTarget->GetFramebuffer(),
             .renderArea      = {
                .offset = { 0, 0 },
                .extent = { width, height }
                },
		     .clearValueCount = clearValueSize,
             .pClearValues    = (const VkClearValue *)pClearValues,
         };

        BeginRenderPass(&beginInfo, VK_SUBPASS_CONTENTS_INLINE);
    }

    // VK_KHR_maintenance1
    VkViewport viewport = {
        .x        = 0,
        .y        = (float)height - 0,
	    .width    = (float)width,
	    .height   = -(float)height,
        .minDepth = 0,
        .maxDepth = 1.0f,
    };

	SetViewport(0, 1, &viewport);
	SetScissor(width, height);
}

void CommandBuffer::EndRenderTarget()
{
	if (true /*dynamic rendering*/)
    {
		vkCmdEndRenderingKHR(handle);
        PipelineImageBarrier(
		    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		    colorImageBarriers[0].newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR ? VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT : VK_PIPELINE_STAGE_TRANSFER_BIT,
		    colorImageBarriers.data(),
		    uint32_t(colorImageBarriers.size())
        );

        if (!depthImageBarriers.empty())
        {
			PipelineImageBarrier(
			    VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
			    VK_PIPELINE_STAGE_TRANSFER_BIT,
			    depthImageBarriers.data(),
			    uint32_t(depthImageBarriers.size())
            );
        }
		colorImageBarriers.resize(0);
		depthImageBarriers.resize(0);
    }
    else
    {
		EndRenderPass();
    }
}

void CommandBuffer::GenerateMipMaps(SuperTexture *_texture, Filter _filter)
{
	VkFilter filter = VK_FILTER_NEAREST;
    if (_filter == Filter::Linear)
    {
		filter = VK_FILTER_LINEAR;
    }

	Texture *texture = InterpretAs<Texture>(_texture);
	uint32_t mipLevels = texture->GetMipLevels();
    if (mipLevels <= 1)
    {
		return;
    }

    const auto &[width, height, depth] = texture->GetExtent();
    VkImageSubresourceRange imageSubresourceRange = {
        .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel   = 0,
        .levelCount     = texture->GetMipLevels(),
	    .baseArrayLayer = 0,
        .layerCount     = texture->GetArrayLayers()
    };

    ImageBarrier barrier{
        *texture,
		imageSubresourceRange,
		texture->GetLayout(),
	    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
	    0,
		VK_ACCESS_TRANSFER_READ_BIT
    };

    PipelineImageBarrier(
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        &barrier
    );

    for (uint32_t i = 1; i < mipLevels; i++)
    {
		uint32_t srcMipLevel = i - 1;
        VkImageBlit imageBlit = {
            .srcSubresource = {
                .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel       = srcMipLevel,
                .baseArrayLayer = 0,
			    .layerCount     = texture->GetArrayLayers(),
            },
            .srcOffsets = {{},
                {
                    .x = int32_t(width  >> srcMipLevel),
                    .y = int32_t(height >> srcMipLevel),
                    .z = 1
                },
            },
            .dstSubresource = {
                .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel       = i,
                .baseArrayLayer = 0,
			    .layerCount     = texture->GetArrayLayers(),
            },
		    .dstOffsets = {{},
            {
                .x = int32_t(width  >> i),
                .y = int32_t(height >> i),
                .z = 1,
            }}
        };

        VkImageSubresourceRange imageSubresourceRange = {
            .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel   = i,
            .levelCount     = 1,
	        .baseArrayLayer = 0,
            .layerCount     = texture->GetArrayLayers()
        };

        ImageBarrier barrier{
            *texture,
		    imageSubresourceRange,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            0,
            VK_ACCESS_TRANSFER_WRITE_BIT
        };

        PipelineImageBarrier(
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            &barrier
        );

        BlitImage(
		    *texture,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		    *texture,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &imageBlit,
		    filter
        );

        barrier.Swap();
		barrier.To(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
        barrier.To(VK_ACCESS_TRANSFER_READ_BIT);

        PipelineImageBarrier(
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            &barrier
        );
    }

    {
        VkImageSubresourceRange imageSubresourceRange = {
            .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel   = 0,
            .levelCount     = texture->GetMipLevels(),
	        .baseArrayLayer = 0,
            .layerCount     = texture->GetArrayLayers()
        };

        ImageBarrier barrier{
            *texture,
		    imageSubresourceRange,
	        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
	        texture->GetLayout(),
	        VK_ACCESS_TRANSFER_READ_BIT,
            0
        };

        PipelineImageBarrier(
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            &barrier
        );
    }
}

void CommandBuffer::CopyBufferToImage(SuperTexture *_texture, uint32_t subresource, SuperBuffer *_buffer, size_t bufferRowLength, uint32_t offset)
{
	Texture *texture = InterpretAs<Texture>(_texture);
    Buffer  *buffer  = InterpretAs<Buffer>(_buffer);
    std::vector<VkBufferImageCopy> bufferCopyRegions;

    uint32_t arrayLayers = texture->Image::GetArrayLayers();
	uint32_t mipLevels   = texture->Image::GetMipLevels();

    Format format = texture->GetFormat();
    auto &[width, height, depth] = texture->GetExtent();
	bufferCopyRegions.emplace_back(VkBufferImageCopy{
	    .bufferOffset      = offset,
		.bufferRowLength   = uint32_t(bufferRowLength / format.GetTexelSize()),
		.bufferImageHeight = 0,
        .imageSubresource  = {
            .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
			.mipLevel       = 0,
            .baseArrayLayer = 0,
            .layerCount     = arrayLayers,
        },
		.imageOffset       = { 0, 0, 0 },
        .imageExtent       = { width, height, depth }
    });

    VkImageSubresourceRange subresourceRange{
        .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel   = 0,
        .levelCount     = mipLevels,
        .baseArrayLayer = 0,
        .layerCount     = arrayLayers,
    };

    ImageBarrier barrier{
        *texture,
        subresourceRange,
	    texture->GetLayout(),
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        0,
        VK_ACCESS_TRANSFER_WRITE_BIT
    };

    PipelineImageBarrier(
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        &barrier
        );

    CopyBufferToImage(
	    *buffer,
		*texture,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        (uint32_t)(bufferCopyRegions.size()),
        bufferCopyRegions.data()
        );

	texture->SetLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	barrier.Swap();
	barrier.To(texture->GetLayout());
    PipelineImageBarrier(
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        &barrier
        );
}

void CommandBuffer::CopyImageToBuffer(SuperBuffer *_buffer, SuperTexture *_texture, uint32_t subresource, size_t bufferRowLength, const Rect2D *pRect)
{
	Texture *texture = InterpretAs<Texture>(_texture);
    Buffer  *buffer  = InterpretAs<Buffer>(_buffer);

    uint32_t arrayLayers = texture->Image::GetArrayLayers();
	uint32_t mipLevels   = texture->Image::GetMipLevels();

	Format format = texture->GetFormat();
	auto &[width, height, depth] = texture->GetExtent();
	VkBufferImageCopy2 copyRegion {
		.sType             = VK_STRUCTURE_TYPE_BUFFER_IMAGE_COPY_2,
		.pNext             = nullptr,
		.bufferOffset      = 0,
		.bufferRowLength   = uint32_t(bufferRowLength / format.GetTexelSize()),
		.bufferImageHeight = texture->GetHeight(),
		.imageSubresource  = {
		    .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
		    .mipLevel       = subresource,
		    .baseArrayLayer = 0,
		    .layerCount     = arrayLayers,
		},
		.imageOffset = {
			.x = 0,
			.y = 0,
			.z = 0
		},
		.imageExtent = {
			.width  = width,
			.height = height,
			.depth  = depth
		}
	};

	if (pRect)
	{
		copyRegion.imageOffset = {
			.x = int(pRect->left),
			.y = int(pRect->top),
			.z = 0,
		};

		copyRegion.imageExtent = {
			.width  = pRect->right - pRect->left,
		    .height = pRect->bottom - pRect->top,
			.depth  = depth,
		};
	}

	VkCopyImageToBufferInfo2 copyInfo{
		.sType          = VK_STRUCTURE_TYPE_COPY_IMAGE_TO_BUFFER_INFO_2,
		.pNext          = nullptr,
		.srcImage       = *texture,
	    .srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		.dstBuffer      = *buffer,
		.regionCount    = 1,
		.pRegions       = &copyRegion,
	};

    VkImageSubresourceRange subresourceRange{
        .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel   = subresource,
        .levelCount     = mipLevels,
        .baseArrayLayer = 0,
        .layerCount     = arrayLayers,
    };

    ImageBarrier barrier{
        *texture,
        subresourceRange,
	    texture->GetLayout(),
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        0,
        VK_ACCESS_TRANSFER_READ_BIT
    };

    PipelineImageBarrier(
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        &barrier
        );

    CopyImageToBuffer2(&copyInfo);
	texture->SetLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	barrier.Swap();
	barrier.To(texture->GetLayout());
    PipelineImageBarrier(
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        &barrier
        );
}

void CommandBuffer::MemoryCopy(SuperBuffer *_buffer, uint32_t size, const void *data, uint32_t offset)
{
	SLASSERT(false && "Don't call this function for Vulkan backend!");
}

void CommandBuffer::MemoryCopy(SuperTexture *texture, const void *data, uint32_t width, uint32_t height, uint32_t rowPitch)
{
	SLASSERT(false && "Don't call this function for Vulkan backend!");
}

void CommandBuffer::MemoryCopy(SuperBuffer *_dst, uint32_t dstOffset, SuperBuffer *_src, uint32_t srcOffset, size_t size)
{
	Buffer *src = InterpretAs<Buffer>(_src);
	Buffer *dst = InterpretAs<Buffer>(_dst);

	//VkBufferCopy2 copyRegion = {
	//	.sType     = VK_STRUCTURE_TYPE_BUFFER_COPY_2,
	//	.pNext     = nullptr,
	//	.srcOffset = srcOffset,
	//	.dstOffset = dstOffset,
	//    .size      = size,
	//};

	//VkCopyBufferInfo2 copyInfo{
	//	.sType       = VK_STRUCTURE_TYPE_COPY_BUFFER_INFO_2,
	//	.pNext       = nullptr,
	//	.srcBuffer   = *src,
	//	.dstBuffer   = *dst,
	//    .regionCount = 1,
	//	.pRegions    = &copyRegion
	//};
	//CopyBuffer2(&copyInfo);

	VkBufferCopy copyRegion{
	    .srcOffset = srcOffset,
		.dstOffset = dstOffset,
		.size      = size,
	};

	CopyBuffer(*src, *dst, 1, &copyRegion);
}

void CommandBuffer::SubmitCommandBuffer(SuperCommandBuffer *secondaryCommandBuffer)
{
	CommandBuffer *commandBuffer = InterpretAs<CommandBuffer>(secondaryCommandBuffer);

    VkCommandBuffer commandBuffers[] = { *commandBuffer };
	ExecuteCommands(1, commandBuffers);
}

void CommandBuffer::DrawInstanced(uint32_t vertexCountPerInstance, uint32_t instanceCount, uint32_t startVertexLocation, uint32_t startInstanceLocation)
{
	Draw(vertexCountPerInstance, instanceCount, startVertexLocation, startInstanceLocation);
}

void CommandBuffer::DrawIndexedInstance(uint32_t indexCountPerInstance, uint32_t instanceCount, uint32_t startIndexLocation, int32_t baseVertexLocation, uint32_t startInstanceLocation)
{
	DrawIndexed(indexCountPerInstance, instanceCount, startIndexLocation, baseVertexLocation, startInstanceLocation);
}

void CommandBuffer::Dispatch(uint32_t nGroupX, uint32_t nGroupY, uint32_t nGroupZ)
{
	_Dispatch(nGroupX, nGroupY, nGroupZ);
}

void CommandBuffer::DispatchMeshTasks(uint32_t nGroupX, uint32_t nGroupY, uint32_t nGroupZ)
{
	DrawMeshTasksEXT(nGroupX, nGroupY, nGroupZ);
}

void CommandBuffer::DispatchRays(const DeviceAddressRegion *rayGenerationShaderRecord, const DeviceAddressRegion *missShaderTable, const DeviceAddressRegion *hitGroupTable, const DeviceAddressRegion *callableShaderTable, uint32_t width, uint32_t height, uint32_t depth)
{
	TraceRaysKHR((const VkStridedDeviceAddressRegionKHR *)rayGenerationShaderRecord,
	             (const VkStridedDeviceAddressRegionKHR *)missShaderTable,
	             (const VkStridedDeviceAddressRegionKHR *)hitGroupTable,
	             (const VkStridedDeviceAddressRegionKHR *)callableShaderTable,
	             width, height, depth);
}

void CommandBuffer::SetImageLayout(SuperTexture *_texture, ImageLayout layout, PipelineStage from, PipelineStage to, const SubresourceRange *pSubresourceRange)
{
	Texture *texture = InterpretAs<Texture>(_texture);
    
    VkImageLayout oldLayout = texture->GetLayout();
	VkImageLayout newLayout = CAST(layout);

	texture->SetLayout(newLayout);
	VkImageMemoryBarrier barrier = {
		.sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.pNext               = nullptr,
		.oldLayout           = oldLayout,
		.newLayout           = newLayout,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image               = *texture,
		.subresourceRange = {
		    .aspectMask     = texture->GetAspectMask(),
			.baseMipLevel   = pSubresourceRange->baseMipLevel,
			.levelCount     = pSubresourceRange->levelCount > 0 ? pSubresourceRange->levelCount : texture->GetMipLevels(),
			.baseArrayLayer = pSubresourceRange->baseMipLevel,
			.layerCount     = pSubresourceRange->layerCount > 0 ? pSubresourceRange->layerCount : texture->GetArrayLayers()
		}
	};

	switch (oldLayout)
	{
		case VK_IMAGE_LAYOUT_UNDEFINED:
			barrier.srcAccessMask = 0;
			break;

		case VK_IMAGE_LAYOUT_GENERAL:
			barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_PREINITIALIZED:
			barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			break;

		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
			break;
		default:
			break;
	}

	switch (newLayout)
	{
		case VK_IMAGE_LAYOUT_GENERAL:
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			break;

		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			barrier.dstAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			if (barrier.srcAccessMask == 0)
			{
				barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
			}
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			break;
		default:
			break;
	}

	PipelineBarrier(
	    CAST(from),
	    CAST(to),
	    0,
	    0, nullptr,
	    0, nullptr,
	    1, &barrier);
}

}
}
