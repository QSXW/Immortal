#pragma once

#include "RenderPass.h"
#include "RenderTask.h"
#include "Graphics/AsyncCompute.h"
#include "Graphics.h"
#include <entt/entt.hpp>
#include <unordered_map>

namespace Immortal
{

struct RenderTargetProperty : public IObject
{
	Ref<RenderTarget> renderTarget;
	std::vector<ClearValue> clearValues;
};

struct RenderTargetCreateInfo
{
	std::string name;

	uint32_t width;
	uint32_t height;

	std::vector<Format> colorFormats;
	Format depthFormat;
	std::vector<ClearValue> clearValues;

	uint32_t sampleCount = 1;
};

class FrameGraph : public IObject, public IClass
{
public:
    FrameGraph();

    ~FrameGraph();

	RenderPass &AddPass(const std::string &name);

	RenderPass *FindPass(const std::string &name);

	void Clear();

	void ClearPasses();

	void Build(AsyncComputeThread *asyncComputeThread = Graphics::GetAsyncComputeThread());

	void Run(CommandBuffer *commandBuffer, const SceneParameters &params, entt::registry &registry);
	
	Ref<RenderTargetProperty> QueryRenderTarget(const std::string &name);

	void AddRenderTarget(std::initializer_list<RenderTargetCreateInfo> &&infos);

	void OnFrameGraphDebugGui();

protected:
	std::vector<RenderPass> passes;

	std::unordered_map<std::string, Ref<RenderTargetProperty>> targets;

    bool hasBuild = false;
};

}
