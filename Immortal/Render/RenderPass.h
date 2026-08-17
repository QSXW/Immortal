#pragma once

#include "Core.h"
#include "Graphics/Types.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace Immortal
{

class RenderTask;
class RenderTarget;
class Texture;

enum class RenderPassPhase
{
	Execute,
	DrawMesh,
	Composite,
};

struct RenderPass
{
	std::string name;
	Ref<RenderTask> task;
	RenderPassPhase phase = RenderPassPhase::Execute;

	Ref<RenderTarget> renderTarget;

	std::vector<ClearValue> clearValues;

	std::unordered_map<std::string, Ref<Texture>> inputs;
	std::unordered_map<std::string, Ref<Texture>> outputs;

	bool isShadowDepthPass = false;

	uint32_t shadowCascadeIndex = 0xFFFFFFFFu;

	/** When false, DrawMesh iterates Transform+Mesh+Material only (e.g. shadow depth). */
	bool filterDrawMeshByTag = true;

	/** SceneLightingModel index for mesh draw when not isGBufferPass. */
	uint32_t lightingMode = 0;

	/** Deferred geometry: MeshletTask uses G-buffer pipeline instead of lightingMode. */
	bool isGBufferPass = false;

	bool useDepthBias = false;
	float depthBiasConstant = 0.0f;
	float depthBiasClamp    = 0.0f;
	float depthBiasSlope    = 0.0f;

	/** When set, ends the active render target before this pass (e.g. compute that must not run inside dynamic rendering). */
	bool endRenderPassBeforeExecute = false;
};

}
