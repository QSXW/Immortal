#pragma once

#include "Core.h"
#include "Format.h"
#include "Interface/IObject.h"
#include "Types.h"

namespace Immortal
{

class IMMORTAL_API Buffer : public IObject
{
public:
    using Type = BufferType;

public:
	Buffer();

	Buffer(BufferType Type, size_t size);

    virtual ~Buffer() = default;

    /**
	 * @brief Get the backend handle
	 *
	 * Vulkan: VkBuffer
	 * OpenGL: GLuint identifier
	 * D3D11:  A pointer to ID3D11Buffer
	 * D3D12:  A pointer to ID3D12Resource
	 */
	virtual Anonymous GetBackendHandle() const = 0;

    virtual void Map(void **ppData, size_t size, uint64_t offset) = 0;

    virtual void Unmap() = 0;

    const size_t &GetSize() const;

	const Type &GetType() const;

protected:
	size_t _size;

	BufferType _type;
};

using SuperBuffer = Buffer;

namespace Interface
{
    using Buffer = SuperBuffer;
}

}
