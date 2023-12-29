#pragma once

#include "Render/AccelerationStructure.h"
#include "Handle.h"

namespace Immortal
{
namespace D3D12
{

class Buffer;
class Device;
struct AccelerationStructureHandle
{

};

class IMMORTAL_API AccelerationStructure : public SuperAccelerationStructure, public NonDispatchableHandle
{
public:
	AccelerationStructure();

	AccelerationStructure(Device *context, const Buffer *pVertexBuffer, const InputElementDescription &desc, const Buffer *pIndexBuffer, const Buffer *pTranformBuffer);

	~AccelerationStructure();

protected:
	URef<Buffer> bottomLevelAS;
	URef<Buffer> topLevelAS;
};

}
}
