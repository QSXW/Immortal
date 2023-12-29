#pragma once

#include "Common.h"
#include "Algorithm/LightArray.h"
#include "Render/RenderTarget.h"
#include "Render/Pipeline.h"
#include "Render/Shader.h"
#include "Handle.h"

namespace Immortal
{
namespace D3D11
{

class Device;
class Shader;
class IMMORTAL_API Pipeline : public virtual SuperPipeline, public NonDispatchableHandle
{
public:
    using Super = SuperPipeline;

public:
    Pipeline(Device *device = nullptr);

    virtual ~Pipeline() override;

    virtual void SetPiplineState(ID3D11DeviceContext4 *commandBuffer) = 0;

public:
	const D3D_PUSH_CONSTANT_DESC &GetPushConstantDesc(uint32_t stage) const
    {
		return pushConstants[stage];
    }
    
    void Swap(Pipeline &other)
    {
		NonDispatchableHandle::Swap(other);
		std::swap(pushConstants, other.pushConstants);
    }

protected:
	LightArray<D3D_PUSH_CONSTANT_DESC, PipelineStage::MaxTypes> pushConstants;
};

class IMMORTAL_API GraphicsPipeline : public Pipeline, public SuperGraphicsPipeline
{
public:
    using Super = SuperGraphicsPipeline;

    using Primitive = ID3D11RasterizerState;
	D3D11_OPERATOR_HANDLE()
	D3D11_SWAPPABLE(GraphicsPipeline)

public:
	GraphicsPipeline(Device *device = nullptr);

    virtual ~GraphicsPipeline() override;

    virtual void Construct(SuperShader **ppShader, size_t shaderCount, const InputElementDescription &description, const std::vector<Format> &outputDescription) override;

    virtual void SetPiplineState(ID3D11DeviceContext4 *commandBuffer) override;

    void ConstructInputLayout(const InputElementDescription &description, const void *pByteCodes, uint32_t byteCodesSize);

    void ConstructRasterizerState();

    void SetBlendFactor(ID3D11DeviceContext4 *commandBuffer, const float *factor);

public:
    uint32_t GetStride() const
    {
		return stride;
    }

    void Swap(GraphicsPipeline &other)
    {
		Pipeline::Swap(other);
		std::swap(handle,            other.handle           );
		std::swap(topology,          other.topology         );
		std::swap(blendState,        other.blendState       );
		std::swap(depthStencilState, other.depthStencilState);
		std::swap(inputLayout,       other.inputLayout      );
		std::swap(vertexShader,      other.vertexShader     );
		std::swap(pixelShader,       other.pixelShader      );
		std::swap(stride,            other.stride           );
    }

protected:
	D3D11_PRIMITIVE_TOPOLOGY topology;

    ComPtr<ID3D11BlendState> blendState;

    ComPtr<ID3D11DepthStencilState> depthStencilState;

	ComPtr<ID3D11InputLayout> inputLayout;

    ComPtr<ID3D11VertexShader> vertexShader;

	ComPtr<ID3D11PixelShader> pixelShader;

    uint32_t stride;
};

class ComputePipeline : public Pipeline, public SuperComputePipeline
{
public:
    using Super = SuperComputePipeline;

    using Primitive = ID3D11ComputeShader;
	D3D11_OPERATOR_HANDLE()
	D3D11_SWAPPABLE(ComputePipeline)

public:
    ComputePipeline(Device *device = nullptr, SuperShader *shader = nullptr);

    virtual ~ComputePipeline() override;

	virtual void SetPiplineState(ID3D11DeviceContext4 *commandBuffer) override;

public:
    void Swap(ComputePipeline &other)
    {
		Pipeline::Swap(other);
		std::swap(handle, other.handle);
    }
};

}
}
