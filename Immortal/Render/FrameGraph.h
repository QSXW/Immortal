#pragma once

#include "RenderTask.h"
#include "Graphics/AsyncCompute.h"
#include "Graphics.h"
#include "Scene/entt.hpp"

namespace Immortal
{

class FrameGraph : public IObject
{
public:
    FrameGraph();

    ~FrameGraph();

	void Build(AsyncComputeThread *asyncComputeThread = Graphics::GetAsyncComputeThread());

	void AddTask(const Ref<RenderTask> &task, const std::string &dependency = {});

	void Execute(CommandBuffer *commandBuffer, const SceneParameters &params);

    void Composite(CommandBuffer *commandBuffer, const SceneParameters &params);

    void DrawMesh(CommandBuffer *commandBuffer, const SceneParameters &params, entt::registry &registry);

protected:
	std::vector<Ref<RenderTask>> tasks;

    bool hasRecorded = false;

    bool hasBuild = false;
};

}
