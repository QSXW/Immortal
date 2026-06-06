#pragma once

#include "Graphics.h"
#include "Graphics/RenderTarget.h"
#include "Shared/IObject.h"
#include "Math/Vector.h"

namespace Immortal
{

class SelectionOutlineTask : public IObject
{
public:
	SelectionOutlineTask();

	~SelectionOutlineTask();

	void Build(AsyncComputeThread *asyncComputeThread);

	void Execute(CommandBuffer *commandBuffer, Texture *sceneColor, Texture *objectIdTexture);

	bool IsReady() const
	{
		return built && pipeline && descriptorSet && outputTarget;
	}

	void SetSelectedObjectId(uint32_t id)
	{
		selectedObjectId = id;
	}

	uint32_t GetSelectedObjectId() const
	{
		return selectedObjectId;
	}

	/** UINT32_MAX = highlight all sub-meshes of the entity; otherwise highlight only the given sub-mesh index. */
	void SetSelectedSubMesh(uint32_t index)
	{
		selectedSubMesh = index;
	}

	uint32_t GetSelectedSubMesh() const
	{
		return selectedSubMesh;
	}

	void SetOutlineColor(const Vector3 &color)
	{
		outlineColor = color;
	}

	const Vector3 &GetOutlineColor() const
	{
		return outlineColor;
	}

	void SetThickness(float value)
	{
		thickness = value;
	}

	float GetThickness() const
	{
		return thickness;
	}

	Ref<RenderTarget> GetOutputTarget() const
	{
		return outputTarget;
	}

	void SetViewportSize(const Vector2 &size);

	void SetEnabled(bool value) { enabled = value; }
	bool GetEnabled() const { return enabled; }

private:
	Ref<GraphicsPipeline> pipeline;

	Ref<DescriptorSet> descriptorSet;

	Ref<Sampler> sampler;

	Ref<RenderTarget> outputTarget;

	uint32_t selectedObjectId = 0;

	uint32_t selectedSubMesh = UINT32_MAX;

	Vector3 outlineColor{ 1.0f, 0.65f, 0.0f };

	float thickness = 1.5f;

	bool built = false;

	bool enabled = true;
};

}
