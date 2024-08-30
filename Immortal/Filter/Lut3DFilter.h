/**
 * Copyright (C) 2024, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#pragma once

#include "Filter.h"

namespace Immortal
{

class Lut3DFilter : public FilterNode
{
public:
	enum class Type
	{
		Nearest,
		Trilinear
	};

public:
	Lut3DFilter(Device *device, const String &filepath, Type type, uint32_t width, uint32_t height);

	virtual ~Lut3DFilter() override;

	void Run(const std::vector<Ref<Texture>> &input, AsyncComputeThread *asyncComputeThread = Graphics::GetAsyncComputeThread()) override;

protected:
	Ref<DescriptorSet> descriptorSet;

	Ref<Pipeline> pipeline;

    Ref<Buffer> lut;

    Ref<Buffer> stagingLut;

    Type type;

    int lutSize;
};

}
