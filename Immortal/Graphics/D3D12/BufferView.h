#pragma once

#include "Common.h"
#include "Graphics/BufferView.h"
#include "Format.h"
#include "Descriptor.h"
#include "Handle.h"

namespace Immortal
{
namespace D3D12
{

class Device;
class Buffer;
class DescriptorHeap;
class BufferView : public SuperBufferView, public NonDispatchableHandle
{
public:
    BufferView(Device *device, Buffer *buffer, Format format);

    BufferView(Device *device, Buffer *buffer, uint32_t byteStride);

    virtual ~BufferView() override;

public:
	void Create(Buffer *buffer, Format format, uint32_t byteStride);

public:
    Descriptor GetDescriptor() const
    {
        return descriptor;
    }

public:
    Descriptor descriptor;

    DescriptorHeap *descriptorHeap;
};

}
}
