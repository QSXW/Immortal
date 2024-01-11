#pragma once

#include "Core.h"

#include "Buffer.h"
#include "Descriptor.h"
#include "Shader.h"
#include "RenderTarget.h"
#include "InputElement.h"
#include "Shared/IObject.h"

namespace Immortal
{

class GraphicsPipeline;
class ComputePipeline;
class IMMORTAL_API Pipeline : public IObject
{
public:
    enum State : uint32_t
    {
        Depth = BIT(0),
        Blend = BIT(1),
    };

    enum class PrimitiveType
    {
        Point,
        Line,
        Triangles
    };

    using Graphics = GraphicsPipeline;
    using Compute = ComputePipeline;

public:
	virtual ~Pipeline() = default;

public:
    void Enable(uint32_t request)
    {
        flags |= request;
    }

    void Disable(uint32_t request)
    {
        flags &= ~request;
    }

protected:
    uint32_t flags = State::Depth;
};

class IMMORTAL_API GraphicsPipeline : public virtual Pipeline
{
public:
    virtual ~GraphicsPipeline() = default;

    virtual void Construct(Shader **ppShader, size_t shaderCount, const InputElementDescription &description, const std::vector<Format> &outputDescription) = 0;
};

class IMMORTAL_API ComputePipeline : public virtual Pipeline
{
public:
	virtual ~ComputePipeline() = default;
};

using SuperPipeline         = Pipeline;
using SuperComputePipeline  = ComputePipeline;
using SuperGraphicsPipeline = GraphicsPipeline;

namespace Interface
{
    using Pipeline         = SuperPipeline;
    using ComputePipeline  = SuperComputePipeline;
    using GraphicsPipeline = SuperGraphicsPipeline;
}

SL_ENABLE_BITWISE_OPERATOR(Pipeline::State)

}
