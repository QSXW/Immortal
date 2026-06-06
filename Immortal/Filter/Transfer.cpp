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
    if (format == Format::Y210)
    {
        formats[kLuma] = Format::RGBA16;
    }
    else if (format.IsType(Format::HightBitDepth))
    {
        formats[kLuma]   = Format::R16;
        formats[kCbCr  ] = Format::RG16;
    }
    else
    {
        formats[kLuma] = Format::R8;
        formats[kCbCr] = Format::RG8;
    }
}

TransferNode::TransferNode() :
    FilterNode{},
    buffer{}
{

}

TransferNode::~TransferNode()
{

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
        
    Format formats[SamplingFactor::kMaxSublayer] = {};
    if (format.IsType(Format::YUV))
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
    for (i = 0; picture[i]; i++)
    {
        data[i].format   = formats[format.IsType(Format::NV) ? i : 0];
        data[i].width    = picture.GetWidth()  >> factors[i].x;
        data[i].height   = picture.GetHeight() >> factors[i].y;
        data[i].rowPitch = SLALIGN(picture.GetStride(i), TextureAlignment);
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
            output.emplace_back(device->CreateTexture(data[i].format, data[i].width, data[i].height, 1, 1, TextureType::TransferDestination));
        }

        if (picture.GetMemoryType() == Vision::PictureMemoryType::System)
        {
            buffer = device->CreateBuffer(totalSize, BufferType::TransferSource);
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

    asyncComputeThread->Execute<RecordingTask>([=, this](uint64_t sync, CommandBuffer *commandBuffer) {
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
                Graphics::MemoryCopyImage(mapped + offset, data[i].rowPitch, picture[i], picture.GetStride(i), data[i].format, data[i].width, data[i].height);
                commandBuffer->CopyBufferToImage(output[i], 0, buffer, data[i].rowPitch, offset);
                offset += data[i].size;
            }
            buffer->Unmap();
        }
    });

    Graphics::Execute<ExecutionCompletedTask>([picture, this]() {});
}

}
