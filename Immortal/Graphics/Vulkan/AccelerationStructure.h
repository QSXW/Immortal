#pragma once

#include "Graphics/AccelerationStructure.h"
#include "Buffer.h"

namespace Immortal
{
namespace Vulkan
{

class AccelerationStructureLevel
{
public:
	using Primitive = VkAccelerationStructureKHR;
	VKCPP_OPERATOR_HANDLE()

public:
	AccelerationStructureLevel();

	AccelerationStructureLevel(Device *device, VkAccelerationStructureTypeKHR type, const VkAccelerationStructureGeometryKHR *pGeometry, uint32_t geometryCount, const uint32_t *pPrimitiveCount);

	AccelerationStructureLevel(AccelerationStructureLevel &&other);

	~AccelerationStructureLevel();

	AccelerationStructureLevel &operator =(AccelerationStructureLevel &&other);

	void Swap(AccelerationStructureLevel &other);

	AccelerationStructureLevel(const AccelerationStructureLevel &other) = delete;

	AccelerationStructureLevel &operator =(const AccelerationStructureLevel &other) = delete;

	void Destroy();

	uint64_t GetDeviceAddress() const
	{
		return deviceAddress;
	}

	uint32_t GetSize() const
	{
		return buffer->GetSize();
	}

	operator uint64_t() const
	{
		return deviceAddress;
	}

public:
	URef<Buffer> buffer;

	uint64_t deviceAddress;
};

class IMMORTAL_API AccelerationStructure : public SuperAccelerationStructure
{
public:


public:
	AccelerationStructure(Device *device = nullptr);

	AccelerationStructure(Device *device, const Buffer *pVertexBuffer, const InputElementDescription &desc, const Buffer *pIndexBuffer, const Buffer *pTranformBuffer);

	~AccelerationStructure();

protected:
	Device *device;

	AccelerationStructureLevel topLevelAS;

	AccelerationStructureLevel bottomLevelAS;
};

}
}
