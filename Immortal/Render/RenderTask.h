#pragma once

#include "Shared/IObject.h"
#include "Math/Vector.h"
#include "Graphics.h"
#include "Mesh.h"
#include "Scene/Component.h"

namespace Immortal
{

struct RenderPass;

struct SceneParameters
{
	static constexpr uint32_t MaxShadowCascades = 4;

	struct
	{
		Vector4 direction;
		Vector4 radiance;
		Vector4 position;
		Vector4 spotParams;
	} lights[4];
	uint32_t lightCount = 0;
	Matrix4 view;
	Matrix4 viewProjection;
	Matrix4 invViewProjection;
	Vector4 cameraWorld;
	Matrix4 skyboxProjection;
	float exposure;
	float gamma;

	/** First directional shadow: light view-projection for depth map (deferred). */
	Matrix4 shadowViewProjection{};
	/** Per-cascade light view-projection (full-resolution depth target each; deferred resolve samples matching texture). */
	Matrix4 shadowCascadeViewProjection[MaxShadowCascades]{};
	float shadowCascadeSplits[MaxShadowCascades]{};
	uint32_t shadowCascadeCount = 0;
	uint32_t shadowEnabled = 0;
	float shadowBias = 0.0015f;
	float shadowStrength = 1.0f;
	/** Forward Phong: 3x3 PCF when 1 (single-map Google-style path); cascades use their own filter. */
	uint32_t shadowEnablePcf = 1u;
	/** Index into lights[] that owns shadowViewProjection (must match deferred resolve). */
	uint32_t shadowCasterLightIndex = 0;
};

class RenderTask : public IObject, public IClass
{
public:
	enum Flags
	{
		MeshRendering = BIT(0),
	};

public:
	RenderTask(const std::string &name, const Flags &flags = {});

	virtual ~RenderTask();

    virtual void Build(AsyncComputeThread *asyncComputeThread) = 0;

	virtual void Execute(CommandBuffer *commandBuffer, const SceneParameters &params) = 0;

    virtual void Composite(CommandBuffer *commandBuffer, const SceneParameters &params) = 0;

	virtual void DrawMesh(
	    CommandBuffer *commandBuffer,
	    const SceneParameters &params,
	    uint32_t objectId,
	    const TransformComponent &transform,
	    MeshComponent &meshComponent,
	    const MaterialComponent &materialComponent,
	    const RenderPass *pass = nullptr)
	{

	}

	/** Editor: ImGui thumbnails / notes for this pass (see FrameGraph::OnFrameGraphDebugGui). */
	virtual void OnFrameGraphDebugGui()
	{
	}

public:
	const Flags &GetFlags() const
	{
		return flags;
	}

public:
    void SetDependency(const Ref<RenderTask> &task);

    const Ref<RenderTask> &GetDependency() const;

    const std::string &GetName() const;

    void SetName(const std::string &value);

protected:
    std::string name;

    Ref<RenderTask> dependency;

	Flags flags;
};
SL_ENABLE_BITWISE_OPERATOR(RenderTask::Flags, uint32_t)

}
