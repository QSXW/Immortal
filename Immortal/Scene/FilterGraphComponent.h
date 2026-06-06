/**
 * Copyright (C) 2024, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#pragma once

#include "Component.h"
#include "Filter/DisplayOrientationFilter.h"
#include "Filter/GaussianBlurFilter.h"
#include "Filter/LogCurveFilter.h"
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

	~FilterGraphComponent();

	template <class T, class... Args>
	void Insert(int index, Args &&...args)
	{
		FilterNode *node = new T{device, std::forward<Args>(args)...};
		nodes.resize(index + 1);
		nodes[index] = std::move(node);
	}

	void Run(const std::vector<Picture> &input, AsyncComputeThread *asyncComputeThread = Graphics::GetAsyncComputeThread());

	void Run(const std::vector<Ref<Texture>> &input, AsyncComputeThread *asyncComputeThread = Graphics::GetAsyncComputeThread());

	const Ref<Texture> &QueryOutput(size_t filterNodeInstance) const;

public:
	Device *device;

	TransferNode transferNode;

	std::vector<Ref<FilterNode>> nodes;

	size_t maxNodeLength;

	std::vector<Ref<Texture>> output;
};

}
