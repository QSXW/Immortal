#include "FrameGraph.h"

namespace Immortal
{

FrameGraph::FrameGraph()
{

}

FrameGraph::~FrameGraph()
{

}

void FrameGraph::Build(AsyncComputeThread *asyncComputeThread)
{
    //asyncComputeThread->Execute(AsyncTaskType::BeginRecording);
    for (auto &task : tasks)
    {
        task->Build(asyncComputeThread);
    }
    //asyncComputeThread->Execute(AsyncTaskType::EndRecording);
    //asyncComputeThread->Execute(AsyncTaskType::Submiting);
    asyncComputeThread->Execute<ExecutionCompletedTask>([=, this] {
        hasBuild = true;
    });
}

void FrameGraph::AddTask(const Ref<RenderTask> &task, const std::string &dependency)
{
    if (tasks.empty())
    {
        tasks.emplace_back(task);
    }
    else if (dependency.empty())
    {
        task->SetDependency(tasks.back());
        tasks.emplace_back(task);
    }
    else
    {
        size_t i = 0;
        for (size_t i = 0; i < tasks.size(); i++)
        {
            if (tasks[i]->GetName() == dependency)
            {
                break;
            }
        }

        if (i == tasks.size())
        {
            throw std::runtime_error("Dependency not matched!");
        }

        task->SetDependency(tasks[i]);
        tasks.emplace_back(task);
    }
}

void FrameGraph::Execute(CommandBuffer *commandBuffer, const SceneParameters &params)
{
    if (!hasBuild)
    {
        return;
    }

	std::string label = "Execute";
	commandBuffer->BeginEvent(label.c_str(), label.size() + 1);
    for (const auto &task : tasks)
    {
        auto &name = task->GetName();
        commandBuffer->BeginEvent(name.c_str(), name.size() + 1);
        task->Execute(commandBuffer, params);
        commandBuffer->EndEvent();
    }
	commandBuffer->EndEvent();

    hasRecorded = true;
}

void FrameGraph::Composite(CommandBuffer *commandBuffer, const SceneParameters &params)
{
    if (!hasBuild)
    {
        return;
    }
	std::string label = "Composite";
	commandBuffer->BeginEvent(label.c_str(), label.size() + 1);
    for (const auto &task : tasks)
    {
        auto &name = task->GetName();
        commandBuffer->BeginEvent(label.c_str(), label.size() + 1);
        task->Composite(commandBuffer, params);
        commandBuffer->EndEvent();
    }
	commandBuffer->EndEvent();
}

void FrameGraph::DrawMesh(CommandBuffer *commandBuffer, const SceneParameters &params, entt::registry &registry)
{
	std::string label = "DrawMesh";
	commandBuffer->BeginEvent(label.c_str(), label.size() + 1);
	for (const auto &task : tasks)
	{
		auto &name = task->GetName();
		commandBuffer->BeginEvent(label.c_str(), label.size() + 1);
		auto view = registry.view<TransformComponent, MeshComponent, MaterialComponent>();
		for (auto object : view)
		{
			auto [transform, mesh, material] = view.get<TransformComponent, MeshComponent, MaterialComponent>(object);
			if (mesh.Mesh)
			{
				task->DrawMesh(commandBuffer, params, (uint32_t)object, transform, mesh.Mesh, material);
            }
		}
		commandBuffer->EndEvent();
	}
	commandBuffer->EndEvent();
}

}
