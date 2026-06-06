/**
 * Copyright (C) 2024, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#pragma once

#include "Filter.h"

namespace Immortal
{

class LogCurveFilterNode : public FilterNode
{
public:
    enum class LogCurveType
    {
        Slog2,
        Slog3,
    };

public:
	LogCurveFilterNode(Device *device, LogCurveType type, uint32_t width, uint32_t height);

    virtual ~LogCurveFilterNode() override;

	void Run(const std::vector<Ref<Texture>> &input, AsyncComputeThread *asyncComputeThread = Graphics::GetAsyncComputeThread()) override;

protected:
	Ref<DescriptorSet> descriptorSet;

	Ref<Pipeline> pipeline;
};

}
