#pragma once

#include "Common.h"
#include "Descriptor.h"
#include "Render/Buffer.h"

namespace Immortal
{
namespace D3D11
{

class Device;
class Buffer : public SuperBuffer
{
public:
    using Super = SuperBuffer;

    using Primitive = ID3D11Buffer;
    D3D11_OPERATOR_HANDLE()

public:
    Buffer() = default;

    Buffer(const Buffer &other, const BindInfo &bindInfo);

    Buffer(Device *device, const size_t size, const void *data, Type type);

    Buffer(Device *device, const size_t size, Type type);
    
    Buffer(Device *device, const size_t size, uint32_t binding);

    virtual ~Buffer() override;

    virtual void Update(uint32_t size, const void *data, uint32_t offset = 0) override;

    virtual Buffer *Bind(const BindInfo &bindInfo) const override;

protected:
	void __Create(const void *data);

protected:
	Device *device;
};

}
}
