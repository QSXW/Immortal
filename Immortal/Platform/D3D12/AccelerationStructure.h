#pragma once

#include "Render/AccelerationStructure.h"
#include "Buffer.h"

namespace Immortal
{
namespace D3D12
{

	class RenderContext;
struct AccelerationStructureHandle
{

};

class IMMORTAL_API AccelerationStructure : public SuperAccelerationStructure
{
public:
	AccelerationStructure();

	AccelerationStructure(RenderContext *context, const Buffer *pVertexBuffer, const InputElementDescription &desc, const Buffer *pIndexBuffer, const Buffer *pTranformBuffer);

	~AccelerationStructure();

protected:
	URef<Buffer> bottomLevelAS;
	URef<Buffer> topLevelAS;

};

}
}
