/**
 * Copyright (C) 2024, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#pragma once

#include "Graphics/LightGraphics.h"
#include "Render/Graphics.h"
#
namespace Immortal
{

class FilterNode : public IObject
{
public:
	SL_ENABLE_COPY(FilterNode)

public:
    FilterNode() :
	    output{},
	    enabled{true}
    {

    }

    virtual ~FilterNode()
    {
        for (auto &o : output)
        {
			Graphics::ReleaseResource(o);
        }
    }

    FilterNode(const FilterNode &other) :
	    output{ other.output },
	    enabled{ other.enabled }
    {

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

    void SetOutput(const std::vector<Ref<Texture>> &value)
    {
		output = value;
    }

	void InvalidateOutput()
	{
		for (auto &texture : output)
		{
			Graphics::ReleaseResource(texture);
		}
		output.clear();
	}

    const bool &Enabled() const
    {
		return enabled;
    }

    void Enabled(const bool &v)
    {
		enabled = v;
    }

public:
    void Swap(FilterNode &other)
    {
		output.swap(other.output);
		std::swap(enabled, other.enabled);
    }

protected:
    std::vector<Ref<Texture>> output;

    bool enabled;
};

}
