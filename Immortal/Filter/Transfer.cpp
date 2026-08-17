/**
 * Copyright (C) 2024, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#include "Transfer.h"

#include <algorithm>
#include <array>

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

    std::array<TransferProxyData, SamplingFactor::kMaxSublayer> data{};
    size_t totalSize = 0;
    size_t planeCount = 0;
    while (planeCount < data.size() && formats[planeCount] != Format::None)
    {
        planeCount++;
    }
    if (!planeCount)
    {
        LOG::ERR("TransferNode cannot determine component formats for {}", format.GetString());
        return;
    }

    size_t texelSize = format.IsType(Format::HightBitDepth) ? 8 : 4;
    for (size_t i = 0; i < planeCount; i++)
    {
        data[i].format = formats[i];
        if (!picture.GetStride(i) ||
            (picture.GetMemoryType() == Vision::PictureMemoryType::System && !picture.GetData(i)))
        {
            LOG::ERR("TransferNode cannot upload plane {} of format {}", i, format.GetString());
            return;
        }
        data[i].width    = picture.GetWidth()  >> factors[i].x;
        data[i].height   = picture.GetHeight() >> factors[i].y;
        const size_t minimumRowPitch = data[i].width * data[i].format.GetTexelSize();
        if (!isRgb && picture.GetMemoryType() == Vision::PictureMemoryType::System &&
            picture.GetStride(i) < minimumRowPitch)
        {
            LOG::ERR(
                "TransferNode plane {} of format {} has stride {}, but {} bytes are required",
                i,
                format.GetString(),
                picture.GetStride(i),
                minimumRowPitch);
            return;
        }
        data[i].rowPitch = SLALIGN(
            isRgb ? data[i].width * texelSize : std::max<size_t>(picture.GetStride(i), minimumRowPitch),
            TextureAlignment);
        data[i].size     = SLALIGN(data[i].rowPitch * data[i].height, 512);
        totalSize += data[i].size;
    }

    uint32_t width  = picture.GetWidth();
    uint32_t height = picture.GetHeight();
    bool recreateOutput = output.size() != planeCount;
    for (size_t i = 0; !recreateOutput && i < planeCount; i++)
    {
        recreateOutput = !output[i] ||
                         output[i]->GetFormat() != data[i].format ||
                         output[i]->GetWidth() != data[i].width ||
                         output[i]->GetHeight() != data[i].height;
    }

    if (recreateOutput)
    {
        auto device = Graphics::GetDevice();
        for (auto &texture : output)
        {
            Graphics::ReleaseResource(texture);
        }
        output.clear();
        output.reserve(planeCount);
        for (size_t i = 0; i < planeCount; i++)
        {
			output.emplace_back(device->CreateTexture(data[i].format, data[i].width, data[i].height, 1, 1, TextureType::TransferDestination | TextureType::Storage));
        }
    }

    std::vector<Ref<Texture>> uploadOutputs = output;
    Ref<Buffer> uploadBuffer;
    if (picture.GetMemoryType() == Vision::PictureMemoryType::System)
    {
        uploadBuffer = Graphics::GetCachedBuffer(BufferType::TransferSource, totalSize);
    }

#ifdef _WIN32
    if (picture.GetMemoryType() == Vision::PictureMemoryType::Device)
    {
        ID3D12Fence *fence = (ID3D12Fence *)picture[1];
        uint64_t     value = (uint64_t)picture[2];
        asyncComputeThread->Execute<QueueTask>([fence, value](Queue *_queue) {
            auto queue = (ID3D12CommandQueue *)_queue->GetBackendHandle();
            if (FAILED(queue->Wait(fence, value)))
            {
                LOG::ERR("Failed to wait fence `{}` and value `{}`", (void *) fence, value);
            }
        });
    }
#endif

    asyncComputeThread->Execute<RecordingTask>([picture, uploadOutputs, uploadBuffer, data, planeCount, totalSize, isRgb, width, height](CommandBuffer *commandBuffer) {
        if (!commandBuffer)
        {
            LOG::ERR("TransferNode upload was recorded without an active command buffer");
            return;
        }
#ifdef _WIN32
        if (picture.GetMemoryType() == Vision::PictureMemoryType::Device)
        {
            for (size_t i = 0; i < planeCount; i++)
            {
                commandBuffer->CopyPlatformSpecificSubresource(uploadOutputs[i], 0, (ID3D12Resource *)picture[0], i);
            }
        }
        else
#endif  
        {
            uint8_t *mapped = nullptr;
            uploadBuffer->Map((void **) &mapped, totalSize, 0);
            size_t offset = 0;
            for (size_t i = 0; i < planeCount; i++)
            {
				auto &planeFormat = data[i].format;
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
					Graphics::MemoryCopyImage(mapped + offset, data[i].rowPitch, picture[i], picture.GetStride(i), planeFormat, data[i].width, data[i].height);
				}
				commandBuffer->CopyBufferToImage(uploadOutputs[i], 0, uploadBuffer, data[i].rowPitch, offset);
                offset += data[i].size;
            }
            uploadBuffer->Unmap();
        }
    });

    asyncComputeThread->Execute<ExecutionCompletedTask>([picture, uploadOutputs, uploadBuffer]() {
        if (uploadBuffer)
        {
            Graphics::ReleaseCachedBuffer(BufferType::TransferSource, uploadBuffer);
        }
    });
}

}
