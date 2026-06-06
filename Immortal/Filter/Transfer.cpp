/**
 * Copyright (C) 2024, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#include "Transfer.h"

namespace Immortal
{

constexpr size_t kLuma = 0;
constexpr size_t kCbCr = 1;

struct TransferProxyData
{
    uint32_t width;
    uint32_t height;
    uint32_t rowPitch;
    size_t   size;
    Format   format;
};

static void FillComponentFormat(Format format, Format *formats)
{
	if (format == Format::Y210 || format == Format::Y216)
	{
		formats[kLuma] = Format::RGBA16;
	}
	else if (format == Format::R8G8B8_UNORM || format == Format::ARGB)
	{
		formats[kLuma] = Format::R8G8B8A8_UNORM;
	}
	else if (format == Format::B8G8R8_UNORM)
	{
		formats[kLuma] = Format::B8G8R8A8_UNORM;
	}
	else if (format.IsType(Format::NV))
    {
		if (format.IsType(Format::HightBitDepth))
	    {
		    formats[kLuma] = Format::R16;
		    formats[kCbCr] = Format::RG16;
	    }
	    else
	    {
		    formats[kLuma] = Format::R8;
		    formats[kCbCr] = Format::RG8;
	    }
    }
    else if (format.IsType(Format::YUV))
    {
		if (format.IsType(Format::HightBitDepth))
		{
			formats[kLuma] = Format::R16;
			formats[1]     = Format::R16;
            formats[2]     = Format::R16;
		}
		else
		{
			formats[kLuma] = Format::R8;
			formats[1]     = Format::R8;
            formats[2]     = Format::R8;
		}
    }
}

TransferNode::TransferNode() :
    FilterNode{},
    buffer{}
{

}

TransferNode::~TransferNode()
{
	Graphics::ReleaseResource(buffer);
}

TransferNode::TransferNode(const TransferNode &other)
{
    output = other.output;
    buffer = other.buffer;
}

void TransferNode::Upload(const Picture &picture, AsyncComputeThread *asyncComputeThread)
{
    auto &format = picture.GetFormat();

    SamplingFactor factors[SamplingFactor::kMaxSublayer] = {};
    GetSamplingFactor(format, factors);
        
    bool isRgb = format == Format::R8G8B8_UNORM ||
        format == Format::B8G8R8_UNORM ||
        format == Format::ARGB;
    Format formats[SamplingFactor::kMaxSublayer] = {};
    if (format == Format::R32G32B32A32_SFLOAT)
    {
        if (output.empty())
        {
            output.resize(1);
        }
        output[0] = Graphics::CreateTexture(
            Format::R32G32B32A32_SFLOAT,
            picture.GetWidth(),
            picture.GetHeight(),
            picture.GetStride(0),
            picture.GetData(),
            asyncComputeThread);
        return;
    }
	if (format.IsType(Format::YUV) || isRgb)
    {
        FillComponentFormat(format, formats);
    }
    else
    {
        if (output.empty())
        {
            output.resize(1);
        }
        output[0] = Graphics::CreateTexture(picture, asyncComputeThread);
        return;
    }

    TransferProxyData data[3] = {};
    size_t totalSize = 0;
    size_t i;
    size_t texelSize = format.IsType(Format::HightBitDepth) ? 8 : 4;
    for (i = 0; picture[i]; i++)
    {
        data[i].format   = formats[format.IsType(Format::NV) ? i : 0];
        data[i].width    = picture.GetWidth()  >> factors[i].x;
        data[i].height   = picture.GetHeight() >> factors[i].y;
		data[i].rowPitch = SLALIGN(isRgb ? data[i].width * texelSize : picture.GetStride(i), TextureAlignment);
        data[i].size     = SLALIGN(data[i].rowPitch * data[i].height, 512);
        totalSize += data[i].size;
    }

    uint32_t width  = picture.GetWidth();
    uint32_t height = picture.GetHeight();
    if (output.empty())
    {
        auto device = Graphics::GetDevice();
        for (size_t i = 0; picture.GetStride(i); i++)
        {
			output.emplace_back(device->CreateTexture(data[i].format, data[i].width, data[i].height, 1, 1, TextureType::TransferDestination | TextureType::Storage));
        }

        if (picture.GetMemoryType() == Vision::PictureMemoryType::System)
        {
			buffer = device->CreateBuffer(BufferType::TransferSource, totalSize);
        }
    }

#ifdef _WIN32
    if (picture.GetMemoryType() == Vision::PictureMemoryType::Device)
    {
        ID3D12Fence *fence = (ID3D12Fence *)picture[1];
        uint64_t     value = (uint64_t)picture[2];
        asyncComputeThread->Execute<QueueTask>([=, this](Queue *_queue) {
            auto queue = (ID3D12CommandQueue *)_queue->GetBackendHandle();
            if (FAILED(queue->Wait(fence, value)))
            {
                LOG::ERR("Failed to wait fence `{}` and value `{}`", (void *) fence, value);
            }
        });
    }
#endif

    asyncComputeThread->Execute<RecordingTask>([=, this](CommandBuffer *commandBuffer) {
#ifdef _WIN32
        if (picture.GetMemoryType() == Vision::PictureMemoryType::Device)
        {
            commandBuffer->CopyPlatformSpecificSubresource(output[0], 0, (ID3D12Resource *)picture[0], 0);
            commandBuffer->CopyPlatformSpecificSubresource(output[1], 0, (ID3D12Resource *)picture[0], 1);
        }
        else
#endif  
        {
            uint8_t *mapped = nullptr;
            buffer->Map((void **) &mapped, totalSize, 0);
            size_t offset = 0;
            for (size_t i = 0; picture[i]; i++)
            {
				auto &format = data[i].format;
				if (isRgb)
				{
					if (picture.GetFormat() == Format::ARGB)
                    {
						for (int y = 0; y < height; y++)
						{
						    uint8_t  *src = &picture.GetData(i)[y * picture.GetStride()];
							uint32_t *dst = (uint32_t *) &mapped[y * data[i].rowPitch];
							for (int x = 0; x < width; x++)
							{
								*dst++ = (src[0] << 24) | (src[3] << 16) | (src[2] << 8) | src[1];
								src += 4;
							}
						}
                    }
					else
                    {
					    for (int y = 0; y < height; y++)
					    {
						    uint8_t  *src = &picture.GetData(i)[y * picture.GetStride()];
						    uint32_t *dst = (uint32_t *)&mapped[y * data[i].rowPitch];
                            for (int x = 0; x < width; x++)
						    {
							    *dst++ = (0xff << 24) | (src[2] << 16) | (src[1] << 8) | src[0];
							    src += 3;
						    }
					    }
                    }
				}
				else
				{
					Graphics::MemoryCopyImage(mapped + offset, data[i].rowPitch, picture[i], picture.GetStride(i), format, data[i].width, data[i].height);
				}
				commandBuffer->CopyBufferToImage(output[i], 0, buffer, data[i].rowPitch, offset);
                offset += data[i].size;
            }
            buffer->Unmap();
        }
    });

    asyncComputeThread->Execute<ExecutionCompletedTask>([picture, this]() {});
}

}
