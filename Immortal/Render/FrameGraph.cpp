#include "FrameGraph.h"

#include "Scene/Component.h"

#include <imgui.h>
#include <unordered_set>

namespace Immortal
{

FrameGraph::FrameGraph() :
	ICLASS
{

}

FrameGraph::~FrameGraph()
{

}

RenderPass &FrameGraph::AddPass(const std::string &name)
{
	passes.push_back({});
	auto &p = passes.back();
	p.name = name;
	return p;
}

RenderPass *FrameGraph::FindPass(const std::string &name)
{
	for (auto &p : passes)
	{
		if (p.name == name)
		{
			return &p;
		}
	}
	return nullptr;
}

void FrameGraph::Clear()
{
	for (auto &[name, target] : targets)
	{
		Graphics::ReleaseResource(target->renderTarget);
		target->renderTarget = {};
	}
	targets.clear();

	passes.clear();
	hasBuild = false;
}

void FrameGraph::ClearPasses()
{
	passes.clear();
	hasBuild = false;
}

void FrameGraph::Build(AsyncComputeThread *asyncComputeThread)
{
	asyncComputeThread->Execute<ExecutionCompletedTask>([=, this] {
		hasBuild = true;
	});
}

void FrameGraph::Run(CommandBuffer *commandBuffer, const SceneParameters &params, entt::registry &registry)
{
	if (!hasBuild)
	{
		return;
	}

	RenderTarget *currentRT = nullptr;

	for (size_t i = 0; i < passes.size(); i++)
	{
		auto &pass = passes[i];
		if (!pass.task)
		{
			continue;
		}

		RenderTarget *wantRT = pass.renderTarget.Get();
		if (pass.endRenderPassBeforeExecute && currentRT)
		{
			commandBuffer->EndRenderTarget();
			currentRT = nullptr;
		}
		else if (wantRT && wantRT != currentRT)
		{
			if (currentRT)
			{
				commandBuffer->EndRenderTarget();
			}
			commandBuffer->BeginRenderTarget(pass.renderTarget, pass.clearValues.empty() ? nullptr : pass.clearValues.data());
			currentRT = wantRT;
		}

		if (pass.useDepthBias)
		{
			commandBuffer->SetDepthBias(pass.depthBiasConstant, pass.depthBiasClamp, pass.depthBiasSlope);
		}

		commandBuffer->BeginEvent(pass.name);
		switch (pass.phase)
		{
		case RenderPassPhase::Execute:
			pass.task->Execute(commandBuffer, params);
			break;

		case RenderPassPhase::Composite:
			pass.task->Composite(commandBuffer, params);
			break;

		case RenderPassPhase::DrawMesh:
		{
			pass.task->Execute(commandBuffer, params);
			auto view = registry.view<TagComponent, TransformComponent, MeshComponent, MaterialComponent>();
			for (auto object : view)
			{
				auto [tag, transform, mesh, material] = view.get<TagComponent, TransformComponent, MeshComponent, MaterialComponent>(object);
				if (mesh.Mesh)
				{
					commandBuffer->BeginEvent(tag.Tag);
					pass.task->DrawMesh(commandBuffer, params, (uint32_t)object, transform, mesh, material, &pass);
					commandBuffer->EndEvent();
				}
			}

			break;
		}
		}

		commandBuffer->EndEvent();

		if (pass.useDepthBias)
		{
			commandBuffer->SetDepthBias(0.0f, 0.0f, 0.0f);
		}
	}

	if (currentRT)
	{
		commandBuffer->EndRenderTarget();
	}
}

Ref<RenderTargetProperty> FrameGraph::QueryRenderTarget(const std::string &name)
{
	auto it = targets.find(name);
	if (it == targets.end())
	{
		CLOG_ERROR("Failed to query render target [{}]", name);
		return {};
	}

	return it->second;
}

void FrameGraph::AddRenderTarget(std::initializer_list<RenderTargetCreateInfo> &&infos)
{
	Device *device = Graphics::GetDevice();
	for (auto &info : infos)
	{
		Ref<RenderTarget> renderTarget = device->CreateRenderTarget(
			info.width,
			info.height,
			info.colorFormats.data(),
			info.colorFormats.size(),
			info.depthFormat,
			info.clearValues.data(),
			info.sampleCount
		);

		renderTarget->SetName(info.name.c_str());

		Ref <RenderTargetProperty> r = new RenderTargetProperty;
		r->renderTarget = renderTarget;
		r->clearValues = info.clearValues;

		targets[info.name] = r;
	}
}

void FrameGraph::OnFrameGraphDebugGui()
{
	if (!hasBuild)
	{
		return;
	}
	for (size_t i = 0; i < passes.size(); i++)
	{
		const auto &pass = passes[i];
		if (!pass.task)
		{
			continue;
		}
		ImGui::PushID((int)i);

		const char *phaseLabel = "?";
		switch (pass.phase)
		{
		case RenderPassPhase::Execute:        phaseLabel = "Execute";        break;
		case RenderPassPhase::DrawMesh:       phaseLabel = "DrawMesh";       break;
		case RenderPassPhase::Composite:      phaseLabel = "Composite";      break;
		}

		std::string label = pass.name + " [" + phaseLabel + "]";
		if (ImGui::TreeNodeEx(label.c_str(), ImGuiTreeNodeFlags_DefaultOpen))
		{
			if (pass.renderTarget)
			{
				ImGui::TextUnformatted("Has render target");
			}
			pass.task->OnFrameGraphDebugGui();
			ImGui::TreePop();
		}
		ImGui::PopID();
	}
}

}
