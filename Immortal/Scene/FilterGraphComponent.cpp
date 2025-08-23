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

FilterGraphComponent::~FilterGraphComponent()
{
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
					if (out->GetMipLevels() > 1)
					{
						asyncComputeThread->Execute<RecordingTask>([=, this](uint64_t sync, CommandBuffer *commandBuffer) {
							commandBuffer->GenerateMipMaps(out, Filter::Linear);
						});
					}
				}
				nextInputs = outputs;
			}
		}
	}

	output = nextInputs;
}

void FilterGraphComponent::Execute(FilterGraphComponent &input, AsyncComputeThread *asyncComputeThread )
{
	Execute(input.output, asyncComputeThread);
}

const Ref<Texture> &FilterGraphComponent::QueryOutput(size_t filterNodeInstance) const
{
	return output[filterNodeInstance];
}

}
