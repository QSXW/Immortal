#pragma once

#include "Graphics.h"
#include "Mesh.h"
#include "RenderTask.h"
#include "Shared/IObject.h"
#include "String/IString.h"
#include "Scene/Component.h"

namespace Immortal
{


struct SceneConstantBuffer
{
	Matrix4  World;
	Matrix4  WorldView;
	Matrix4  WorldViewProj;
	uint32_t DrawMeshlets;
};

struct MeshInfo
{
	Matrix4 Model;
	uint32_t IndexBytes;
	uint32_t MeshletOffset;
};

class MeshletTask : public RenderTask
{
public:
	MeshletTask();

	virtual ~MeshletTask() override;

	virtual void Build(AsyncComputeThread *asyncComputeThread) override;

	virtual void Execute(CommandBuffer *commandBuffer, const SceneParameters &params) override;

	virtual void Composite(CommandBuffer *commandBuffer, const SceneParameters &params) override;

	virtual void DrawMesh(CommandBuffer *commandBuffer, const SceneParameters &params, uint32_t objectId, const TransformComponent &transform, const Ref<Mesh> &mesh, const MaterialComponent &materialComponent) override;

protected:
	Ref<GraphicsPipeline> pipeline;

	Ref<Sampler> sampler;

	Ref<DescriptorSet> descriptorSet;

	Ref<Buffer> stagingBuffer;

	Ref<Buffer> constantBuffer;
};

}
