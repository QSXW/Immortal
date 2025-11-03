/**
 * Copyright (C) 2024, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#pragma once

#include "Filter.h"
#include "Math/Vector.h"

namespace Immortal
{

class ScaleFilter : public FilterNode
{
public:
	ScaleFilter(Device *device, Format srcFormat, Format dstFormat, uint32_t width, uint32_t height, ColorSpace colorSpace = ColorSpace::BT709, bool fullRange = false);

    virtual ~ScaleFilter() override;

    virtual void Run(const std::vector<Ref<Texture>> &input, AsyncComputeThread *asyncComputeThread = Graphics::GetAsyncComputeThread()) override;

protected:
	void CreateOutputs(uint32_t width, uint32_t height);

protected:
	Device *device;

	Ref<DescriptorSet> descriptorSet;

	Ref<Pipeline> pipeline;

    Ref<Sampler> sampler;

    Format srcFormat;

    Format dstFormat;

    ColorSpace colorSpace;

    Matrix4 transform;

    int transformIndex;
};

}
