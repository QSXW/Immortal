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

void FilterGraphComponent::Run(const std::vector<Picture> &input, AsyncComputeThread *asyncComputeThread)
{
	std::vector<Ref<Texture>> nextInputs;
	for (auto &picture : input)
	{
		transferNode.Upload(picture, asyncComputeThread);
		nextInputs.insert(nextInputs.end(), transferNode.GetOutput().begin(), transferNode.GetOutput().end());
	}

	Run(nextInputs, asyncComputeThread);
}

void FilterGraphComponent::Run(const std::vector<Ref<Texture>> &input, AsyncComputeThread *asyncComputeThread)
{
	std::vector<Ref<Texture>> nextInputs = input;

	for (size_t i = 0; i < nodes.size(); i++)
	{
		auto &node = nodes[i];
		std::vector<Ref<Texture>> output;
		node->Preprocess();
		node->Run(nextInputs, asyncComputeThread);
		node->PostProcess();
		
		for (auto &out : node->GetOutput())
		{
			if (out->GetMipLevels() > 1)
			{
				asyncComputeThread->Execute<RecordingTask>([=, this](uint64_t sync, CommandBuffer *commandBuffer) {
					commandBuffer->GenerateMipMaps(out, Filter::Linear);
				});
			}
		}
		nextInputs = node->GetOutput();
	}

	output = nextInputs;
}

const Ref<Texture> &FilterGraphComponent::QueryOutput(size_t filterNodeInstance) const
{
	return output[filterNodeInstance];
}

}
