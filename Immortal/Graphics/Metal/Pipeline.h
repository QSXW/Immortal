#pragma once

#include "Common.h"
#include "Handle.h"
#include "Graphics/RenderTarget.h"
#include "Graphics/Pipeline.h"
#include "Metal/MTLComputePipeline.hpp"
#include "Metal/MTLRenderPipeline.hpp"
#include "Types.h"

namespace Immortal
{
namespace Metal
{

class Device;
class Shader;
class IMMORTAL_API Pipeline : public virtual SuperPipeline
{
public:
    using Super = SuperPipeline;

    enum class Type
    {
        Graphics,
        Compute
    };

    METAL_SWAPPABLE(Pipeline)

public:
	Pipeline(Device *device = nullptr, Type type = Type::Graphics);

    virtual ~Pipeline() override;

public:
    Type GetType() const
    {
		return type;
    }

    void Swap(Pipeline &other)
    {
        std::swap(device, other.device);
        std::swap(type,   other.type  );
    }

protected:
    Device *device;

	Type type;
};

class IMMORTAL_API GraphicsPipeline : public Pipeline, public SuperGraphicsPipeline, public Handle<MTL::RenderPipelineState>
{
public:
    using Super     = SuperGraphicsPipeline;
    METAL_SWAPPABLE(GraphicsPipeline)

public:
    GraphicsPipeline(Device *device = nullptr);

    virtual ~GraphicsPipeline() override;

	virtual void Construct(SuperShader **ppShader, size_t shaderCount, const InputElementDescription &description, const std::vector<Format> &outputDescription) override;

public:
    uint32_t GetPushConstantIndex(PushConstantIndexType type) const
    {
        uint32_t *data = (uint32_t *)&pushConstantIndices;
        return data[(size_t)type];
    }

    void SetPushConstantIndex(PushConstantIndexType index, uint32_t slot)
    {
        uint32_t *data = (uint32_t *)&pushConstantIndices;
        data[(size_t)type] = slot;
    }

    void Swap(GraphicsPipeline &other)
    {
        Pipeline::Swap(other);
        Handle::Swap(other);
        std::swap(pushConstantIndices, other.pushConstantIndices);
    }

protected:
    uint64_t pushConstantIndices;
};

class IMMORTAL_API ComputePipeline : public Pipeline, public SuperComputePipeline, public Handle<MTL::ComputePipelineState>
{
public:
    using Super = SuperComputePipeline;
    METAL_SWAPPABLE(ComputePipeline)

public:
    ComputePipeline(Device *device, SuperShader *shader);

    ComputePipeline(Device *device = nullptr, Shader *shader = nullptr);

    virtual ~ComputePipeline() override;

public:
    const Size3D &GetThreadGroupSize() const
    {
        return threadGroupSize;
    }

    uint32_t GetPushConstantIndex() const
    {
        return pushConstantIndex;
    }

    void Swap(ComputePipeline &other)
    {
        Pipeline::Swap(other);
        Handle::Swap(other);
        std::swap(threadGroupSize,   other.threadGroupSize  );
        std::swap(pushConstantIndex, other.pushConstantIndex);
    }

protected:
    Size3D threadGroupSize;

    uint32_t pushConstantIndex;
};

}
}
