/**
 * Copyright (C) 2024, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#include "FilterGraphComponent.h"
#include "Vision/Common/SamplingFactor.h"

namespace Immortal
{

FilterGraphComponent::FilterGraphComponent(Device *device) :
	device{ device },
    nodes{},
	transferNode{},
    maxNodeLength{}
{

}

FilterGraphComponent::FilterGraphComponent(const FilterGraphComponent &other) :
    device{ other.device },
    nodes{ other.nodes },
    transferNode{ other.transferNode },
    maxNodeLength{ other.maxNodeLength },
    output{ other.output }
{

}

FilterGraphComponent::~FilterGraphComponent()
{
	for (auto &o : output)
	{
		Graphics::ReleaseResource(o);
	}
	nodes = {};
}

void FilterGraphComponent::Execute(AsyncComputeThread *asyncComputeThread)
{
	std::vector<Ref<Texture>> nextInputs;
	nextInputs.insert(nextInputs.end(), transferNode.GetOutput().begin(), transferNode.GetOutput().end());
	Execute(nextInputs, asyncComputeThread);
}

void FilterGraphComponent::Execute(const std::vector<Picture> &input, AsyncComputeThread *asyncComputeThread)
{
	std::vector<Ref<Texture>> nextInputs;
	for (auto &picture : input)
	{
		transferNode.Upload(picture, asyncComputeThread);
		nextInputs.insert(nextInputs.end(), transferNode.GetOutput().begin(), transferNode.GetOutput().end());
	}

	Execute(nextInputs, asyncComputeThread);
}

void FilterGraphComponent::Execute(const std::vector<Ref<Texture>> &input, AsyncComputeThread *asyncComputeThread)
{
	transferNode.SetOutput(input);
	std::vector<Ref<Texture>> nextInputs = input;

	for (size_t i = 0; i < nodes.size(); i++)
	{
		auto &node = nodes[i];
		std::vector<Ref<Texture>> output;

		if (node->Enabled())
		{
			node->Preprocess();
			node->Run(nextInputs, asyncComputeThread);
			node->PostProcess();

			auto &outputs = node->GetOutput();
			if (!outputs.empty())
			{
				for (auto &out : node->GetOutput())
				{
					Ref<Texture> refOutput = out;
					if (out->GetMipLevels() > 1)
					{
						asyncComputeThread->Execute<RecordingTask>([refOutput](CommandBuffer *commandBuffer) {
							commandBuffer->GenerateMipMaps(refOutput, Filter::Linear);
						});
					}
				}
				nextInputs = outputs;
			}
		}
	}

	for (auto &out : nextInputs)
	{
		Ref<Texture> refOutput = out;
		asyncComputeThread->Execute<RecordingTask>([refOutput](CommandBuffer *commandBuffer) {
			commandBuffer->SetImageLayout(
				refOutput,
				ImageLayout::General,
				PipelineStage::ComputeShading,
				PipelineStage::All);
		});
	}

	output = nextInputs;
}

void FilterGraphComponent::Execute(FilterGraphComponent &input, AsyncComputeThread *asyncComputeThread )
{
	Execute(input.output, asyncComputeThread);
}

void FilterGraphComponent::InvalidateRenderedOutput()
{
	for (auto nodeIt = nodes.rbegin(); nodeIt != nodes.rend(); ++nodeIt)
	{
		const auto &nodeOutput = (*nodeIt)->GetOutput();
		bool isRenderedOutput = false;
		for (auto &candidate : nodeOutput)
		{
			if (!candidate)
			{
				continue;
			}
			for (auto &rendered : output)
			{
				if (rendered && candidate.Get() == rendered.Get())
				{
					isRenderedOutput = true;
					break;
				}
			}
			if (isRenderedOutput)
			{
				break;
			}
		}

		if (isRenderedOutput)
		{
			(*nodeIt)->InvalidateOutput();
			return;
		}
	}
}

const Ref<Texture> &FilterGraphComponent::QueryOutput(size_t index) const
{
	if (index >= output.size())
	{
		static Ref<Texture> empty;
		return empty;
	}
	return output[index];
}

const std::vector<Ref<Texture>> &FilterGraphComponent::QueryOutputs() const
{
	return output;
}

}
