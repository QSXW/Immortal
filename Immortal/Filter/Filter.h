/**
 * Copyright (C) 2024, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#pragma once

#include "Graphics/LightGraphics.h"
#include "Render/Graphics.h"

namespace Immortal
{

class FilterNode : public IObject
{
public:
	SL_ENABLE_COPY(FilterNode)

    FilterNode() :
	    output{}
    {

    }

    virtual ~FilterNode()
    {

    }

    FilterNode(const FilterNode &other)
    {
		output = other.output;
    }

    virtual void Preprocess()
    {

    }

    virtual void Run(const std::vector<Ref<Texture>> &input, AsyncComputeThread *asyncComputeThread = Graphics::GetAsyncComputeThread())
    {

    }

    virtual void PostProcess()
    {

    }

    virtual const std::vector<Ref<Texture>> &GetOutput() const
    {
		return output;
    }

public:
    void Swap(FilterNode &other)
    {
		output.swap(other.output);
    }

protected:
    std::vector<Ref<Texture>> output;
};

}
