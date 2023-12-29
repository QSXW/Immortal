#pragma once

#include "Common.h"
#include "Descriptor.h"
#include "Render/Buffer.h"
#include "Handle.h"

namespace Immortal
{
namespace D3D11
{

class Device;
class IMMORTAL_API Buffer : public SuperBuffer, public NonDispatchableHandle
{
public:
    using Super = SuperBuffer;

    using Primitive = ID3D11Buffer;
    D3D11_OPERATOR_HANDLE()
	D3D11_SWAPPABLE(Buffer)

public:
    Buffer();

    Buffer(Device *device, const size_t size, Type type, const void *data = nullptr);

    virtual ~Buffer() override;

    virtual Anonymous GetBackendHandle() const override;

    virtual void Map(void **ppData, size_t size, uint64_t offset) override;

	virtual void Unmap() override;

public:
    uint32_t GetOffset() const
    {
		return 0;
    }

    const Descriptor<SRV> &GetDescriptor() const
    {
		return descriptor;
    }

    void Swap(Buffer &other)
    {
		NonDispatchableHandle::Swap(other);
		std::swap(handle,     other.handle    );
		std::swap(descriptor, other.descriptor);
		std::swap(mapFlags,   other.mapFlags  );
    }

protected:
	void ConstructHandle(const void *data = nullptr);

protected:
    Descriptor<SRV> descriptor;

    uint32_t mapFlags;
};

}
}
