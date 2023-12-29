#include "Pipeline.h"
#include "Device.h"
#include "Buffer.h"
#include "Texture.h"
#include "Shader.h"
#include "RenderTarget.h"
#include "Device.h"

namespace Immortal
{
namespace D3D11
{

static void *NullRef[32] = {};

static inline D3D11_PRIMITIVE_TOPOLOGY CAST(const Pipeline::PrimitiveType type)
{
    switch (type)
    {
    case Pipeline::PrimitiveType::Line:
		return D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;

    case Pipeline::PrimitiveType::Point:
        return D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;

    case Pipeline::PrimitiveType::Triangles:
    default:
        return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    }
}

Pipeline::Pipeline(Device *device) :
    NonDispatchableHandle{ device }
{

}

Pipeline::~Pipeline()
{

}

GraphicsPipeline::GraphicsPipeline(Device *device) :
    Pipeline{ device },
    handle{},
    blendState{},
    depthStencilState{},
	inputLayout{},
    topology{ D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST },
    stride{}
{

}

GraphicsPipeline::~GraphicsPipeline()
{
	vertexShader.Reset();
	pixelShader.Reset();
	inputLayout.Reset();
	handle.Reset();
}

void GraphicsPipeline::Construct(SuperShader **_ppShader, size_t shaderCount, const InputElementDescription &description, const std::vector<Format> &outputDescription)
{
	pushConstants.resize(PipelineStage::MaxTypes);
	Shader **ppShader = (Shader **)_ppShader;
	for (size_t i = 0; i < shaderCount; i++)
	{
		Shader *shader = ppShader[i];		
		if (shader->GetStage() == ShaderStage::Vertex)
		{
			vertexShader = shader->Get<ID3D11VertexShader>();
			
			auto byteCodesBlob = shader->GetByteCodes();
			ConstructInputLayout(description, byteCodesBlob->GetBufferPointer(), byteCodesBlob->GetBufferSize());

			pushConstants[PipelineStage::Vertex] = shader->GetPushConstantDesc();
		}
		else if (shader->GetStage() == ShaderStage::Pixel)
		{
			pixelShader = shader->Get<ID3D11PixelShader>();
			pushConstants[PipelineStage::Pixel] = shader->GetPushConstantDesc();
		}
	}

	ConstructRasterizerState();

	if (flags & State::Blend)
	{
		D3D11_BLEND_DESC blendDesc = {
		    .AlphaToCoverageEnable  = false,
		    .IndependentBlendEnable = true,
		    .RenderTarget           = {}
		};

		for (size_t i = 0; i < outputDescription.size(); i++)
		{
			blendDesc.RenderTarget[i] = {
				.BlendEnable           = true,
				.SrcBlend              = D3D11_BLEND_SRC_ALPHA,
			    .DestBlend             = D3D11_BLEND_INV_SRC_ALPHA,
			    .BlendOp               = D3D11_BLEND_OP_ADD,
			    .SrcBlendAlpha         = D3D11_BLEND_ONE,
			    .DestBlendAlpha        = D3D11_BLEND_INV_SRC_ALPHA,
			    .BlendOpAlpha          = D3D11_BLEND_OP_ADD,
			    .RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL,
			};
		}

		Check(device->CreateBlendState(&blendDesc, &blendState));
	}
	
	if (flags & State::Depth)
	{
		D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {
			.DepthEnable     = false,
			.DepthWriteMask  = D3D11_DEPTH_WRITE_MASK_ALL,
			.DepthFunc       = D3D11_COMPARISON_ALWAYS,
			.StencilEnable   = false,
			.FrontFace       = {
			    .StencilFailOp      = D3D11_STENCIL_OP_KEEP,
			    .StencilDepthFailOp = D3D11_STENCIL_OP_KEEP,
				.StencilPassOp      = D3D11_STENCIL_OP_KEEP,
			    .StencilFunc        = D3D11_COMPARISON_ALWAYS
			},
			.BackFace = depthStencilDesc.FrontFace,
		};
		device->CreateDepthStencilState(&depthStencilDesc, &depthStencilState);
	}
}

void GraphicsPipeline::ConstructInputLayout(const InputElementDescription &description, const void *pByteCodes, uint32_t byteCodesSize)
{
	std::vector<D3D11_INPUT_ELEMENT_DESC> inputElementDesc;
	inputElementDesc.reserve(description.Size());

	for (size_t i = 0; i < description.Size(); i++)
	{
		inputElementDesc.emplace_back(D3D11_INPUT_ELEMENT_DESC{
		    .SemanticName         = description[i].GetSemanticsName().c_str(),
			.SemanticIndex        = 0,
			.Format               = description[i].GetFormat(),
			.InputSlot            = 0,
		    .AlignedByteOffset    = description[i].GetOffset(),
			.InputSlotClass       = D3D11_INPUT_PER_VERTEX_DATA,
			.InstanceDataStepRate = 0,
		});
	}
	stride = description.GetStride();

	Check(device->CreateInputLayout(inputElementDesc.data(), uint32_t(inputElementDesc.size()), pByteCodes, byteCodesSize, &inputLayout));
}

void GraphicsPipeline::ConstructRasterizerState()
{
	D3D11_RASTERIZER_DESC rasterizerDesc = {
		.FillMode              = D3D11_FILL_SOLID,                
	    .CullMode              = D3D11_CULL_NONE,
	    .FrontCounterClockwise = FALSE,            
	    .DepthBias             = 0,                
	    .DepthBiasClamp        = 0,    
	    .SlopeScaledDepthBias  = 0,          
	    .DepthClipEnable       = TRUE,               
	    .ScissorEnable         = TRUE,   
	    .MultisampleEnable     = FALSE,    
	    .AntialiasedLineEnable = FALSE,
    };

    Check(device->CreateRasterizerState(&rasterizerDesc, &handle));
}

void GraphicsPipeline::SetPiplineState(ID3D11DeviceContext4 *commandBuffer)
{
	commandBuffer->IASetPrimitiveTopology(topology);
	commandBuffer->RSSetState(handle.Get());
	commandBuffer->IASetInputLayout(inputLayout.Get());
	commandBuffer->VSSetShader(vertexShader.Get(), nullptr, 0);
	commandBuffer->PSSetShader(pixelShader.Get(), nullptr, 0);
	if (depthStencilState)
	{
		commandBuffer->OMSetDepthStencilState(depthStencilState.Get(), 0);
	}
}

void GraphicsPipeline::SetBlendFactor(ID3D11DeviceContext4 *commandBuffer, const float *factor)
{
	if (blendState)
	{
		commandBuffer->OMSetBlendState(blendState.Get(), factor, 0xffffffff);
	}
}

ComputePipeline::ComputePipeline(Device *device, SuperShader *_shader) :
    Pipeline{ device }
{
	Shader *shader = InterpretAs<Shader>(_shader);
	if (shader->GetStage() != ShaderStage::Compute)
    {
        LOG::ERR("Failed to create compute pipeline with a non compute stage shader");
        return;
    }

	handle = shader->Get<ID3D11ComputeShader>();
	pushConstants[PipelineStage::Compute] = shader->GetPushConstantDesc();
}

ComputePipeline::~ComputePipeline()
{
	handle.Reset();
}

void ComputePipeline::SetPiplineState(ID3D11DeviceContext4 *commandBuffer)
{
	commandBuffer->CSSetShader(handle.Get(), nullptr, 0);
}

}
}
