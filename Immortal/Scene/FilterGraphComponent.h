/**
 * Copyright (C) 2024, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#pragma once

#include "Component.h"
#include "Filter/DisplayOrientationFilter.h"
#include "Filter/GaussianBlurFilter.h"
#include "Filter/Lut3DFilter.h"
#include "Filter/ScaleFilter.h"
#include "Filter/Transfer.h"

namespace Immortal
{

struct FilterGraphComponent : public Component
{
public:
	DEFINE_COMPONENT_TYPE(Filter)

	FilterGraphComponent(Device *device = Graphics::GetDevice());

	FilterGraphComponent(const FilterGraphComponent &other);

	~FilterGraphComponent();

	template <class T, class... Args>
	Ref<FilterNode> &Insert(int index, Args &&...args)
	{
		FilterNode *node = new T(device, std::forward<Args>(args)...);
		nodes.insert(nodes.begin() + index, std::move(node));
		return nodes[index];
	}

	template <class T, class... Args>
	Ref<FilterNode> &Emplace(Args &&...args)
	{
		FilterNode *node = new T(device, std::forward<Args>(args)...);
		nodes.emplace_back(node);
		return nodes.back();
	}

	Ref<FilterNode> GetNode(size_t index)
	{
		return nodes[index];
	}

	template <class T>
	void EmplaceBack(Ref<T> &node)
	{
		nodes.emplace_back(node);
	}

	void Merge(FilterGraphComponent &other)
	{
		nodes.insert(nodes.end(), other.nodes.begin(), other.nodes.end());
	}

	size_t GetNodeSize() const
	{
		return nodes.size();
	}

    void Execute(AsyncComputeThread *asyncComputeThread = Graphics::GetAsyncComputeThread());

	void Execute(const std::vector<Picture> &input, AsyncComputeThread *asyncComputeThread = Graphics::GetAsyncComputeThread());

	void Execute(const std::vector<Ref<Texture>> &input, AsyncComputeThread *asyncComputeThread = Graphics::GetAsyncComputeThread());

	void Execute(FilterGraphComponent &input, AsyncComputeThread *asyncComputeThread = Graphics::GetAsyncComputeThread());

	void InvalidateRenderedOutput();

	const Ref<Texture> &QueryOutput(size_t filterNodeInstance = 0) const;

	const std::vector<Ref<Texture>> &QueryOutputs() const;

public:
	Device *device;

	TransferNode transferNode;

	std::vector<Ref<FilterNode>> nodes;

	size_t maxNodeLength;

	std::vector<Ref<Texture>> output;
};

}
