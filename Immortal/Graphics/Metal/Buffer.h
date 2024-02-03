#pragma once

#include "Common.h"
#include "Handle.h"
#include "Graphics/Buffer.h"
#include <__type_traits/is_swappable.h>

namespace Immortal
{
namespace Metal
{

class Device;
class IMMORTAL_API Buffer : public SuperBuffer, public Handle<MTL::Buffer>
{
public:
    using Super = SuperBuffer;
    METAL_SWAPPABLE(Buffer)

public:
	Buffer();

    Buffer(Device *device, Type type, const size_t size, const void *data = nullptr);

    virtual ~Buffer() override;

    virtual Anonymous GetBackendHandle() const override;

    virtual void Map(void **ppData, size_t size, uint64_t offset) override;

	virtual void Unmap() override;

public:
    void Swap(Buffer &other)
    {
        Handle::Swap(other);
        std::swap(mappedSize, other.mappedSize);
    }

protected:
    size_t mappedSize;
};

}
}
