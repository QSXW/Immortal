#pragma once

#include "Common.h"
#include "Render/Buffer.h"

namespace Immortal
{
namespace OpenGL
{

class IMMORTAL_API Buffer : public SuperBuffer, public Handle
{
public:
    using Super         = SuperBuffer;
    using BindPointType = uint32_t;

public:
    Buffer(uint64_t size, Type type);

    Buffer(uint64_t size, const void *data, Type type);

    virtual ~Buffer() override;

    virtual Anonymous GetBackendHandle() const override;

    virtual void Map(void **ppData, size_t size, uint64_t offset);

	virtual void Unmap();

public:
    void Update(uint64_t size, const void *data, uint64_t offset = 0);

public:
    void Bind() const
    {
        glBindBuffer(bindPoint, handle);
    }

    void Unbind() const
    {
        glBindBuffer(bindPoint, 0);
    }

    const uint8_t *GetMemory() const
    {
		return memory.data();
    }

    BindPointType GetBindPoint() const
    {
		return bindPoint;
    }

protected:
    BindPointType bindPoint{ GL_ARRAY_BUFFER };

    std::vector<uint8_t> memory;

    uint32_t access;
};

}
}
