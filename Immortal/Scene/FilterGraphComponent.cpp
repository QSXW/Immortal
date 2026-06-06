#include "Component.h"

namespace Immortal
{

TransferNode::TransferNode(int nextIndex, Type type) :
    FilterNode{ nextIndex },
	type{ type }
{

}

void TransferNode::Run(const std::vector<Ref<Texture>> &input, std::vector<Ref<Texture>> &output, AsyncComputeThread *asyncComputeThread)
{

}

FilterGraphComponent::~FilterGraphComponent()
{
	for (auto &nodes : nodeGroups)
	{
		for (auto &node : nodes)
		{
			if (node)
			{
				delete node;
				node = nullptr;
			}
		}
	}
}

void FilterGraphComponent::Run(const std::vector<std::vector<Ref<Texture>>> &input, std::vector<Ref<Texture>> &output, AsyncComputeThread *asyncComputeThread)
{
	std::vector<std::vector<Ref<Texture>>> nextInputs = input;

	for (auto &nodes : nodeGroups)
	{
		for (size_t i = 0; i < nodes.size(); i++)
		{
			auto &node = nodes[i];
			std::vector<Ref<Texture>> output;
			node->Preprocess();
			node->Run(nextInputs[i], output, asyncComputeThread);
			node->PostProcess();
			nextInputs[i] = std::move(output);
		}
	}
}

}
