#pragma once

#include "RenderTask.h"
#include "Graphics/AsyncCompute.h"
#include "Graphics.h"

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

protected:
	std::vector<Ref<RenderTask>> tasks;

    bool hasRecorded = false;

    bool hasBuild = false;
};

}
