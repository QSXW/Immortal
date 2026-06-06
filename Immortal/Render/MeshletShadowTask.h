#pragma once

#include "Graphics.h"
#include "Mesh.h"
#include "RenderPass.h"
#include "RenderTask.h"
#include "Scene/Component.h"

namespace Immortal
{

class MeshletShadowTask : public RenderTask, public IClass
{
public:
	MeshletShadowTask();

	~MeshletShadowTask() override;

	void Build(AsyncComputeThread *asyncComputeThread) override;

	void Execute(CommandBuffer *commandBuffer, const SceneParameters &params) override;

	void Composite(CommandBuffer *commandBuffer, const SceneParameters &params) override;

	void DrawMesh(
	    CommandBuffer *commandBuffer,
	    const SceneParameters &params,
	    uint32_t objectId,
	    const TransformComponent &transform,
	    MeshComponent &meshComponent,
	    const MaterialComponent &materialComponent,
	    const RenderPass *pass = nullptr) override;

	void OnFrameGraphDebugGui() override;

protected:
	Ref<GraphicsPipeline> pipeline;
};

}
