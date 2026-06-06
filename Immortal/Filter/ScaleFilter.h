/**
 * Copyright (C) 2024, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#pragma once

#include "Filter.h"

namespace Immortal
{

class ScaleFilter : public FilterNode
{
public:
	ScaleFilter(Device *device, Format dstFormat, Format srcFormat, uint32_t width, uint32_t height);

    virtual ~ScaleFilter() override;

    virtual void Run(const std::vector<Ref<Texture>> &input, AsyncComputeThread *asyncComputeThread = Graphics::GetAsyncComputeThread()) override;

protected:
	Ref<DescriptorSet> descriptorSet;

	Ref<Pipeline> pipeline;

    Format srcFormat;
};

}
