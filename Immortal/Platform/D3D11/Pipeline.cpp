#include "Pipeline.h"
#include "Device.h"
#include "Buffer.h"
#include "Texture.h"
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
    device{ device }
{

}

Pipeline::~Pipeline()
{

}

GraphicsPipeline::GraphicsPipeline(Device *device, Ref<Shader::Super> _shader) :
    Super{_shader},
    Pipeline{ device }
{
	if (!_shader)
	{
		return;
	}

	sampler = new Sampler{ device, {} };

	Topology = CAST(desc.primitiveType);

	auto shader = _shader.InterpretAs<Shader>();
	shader->QueryInterface<ID3D11VertexShader>(&vertexShader);
	shader->QueryInterface<ID3D11PixelShader>(&pixelShader);

	shader->GetDesc(&pushConstants.desc);
	if (pushConstants.desc)
	{
		pushConstants.buffer = new Buffer{ device, pushConstants.desc.Size, Buffer::Type::PushConstant };
	}
}

GraphicsPipeline::GraphicsPipeline(Device *device, Ref<Shader> _shader) :
    Super{_shader},
    Pipeline{ device }
{
	sampler = new Sampler{ device, {} };
	Topology = CAST(desc.primitiveType);
}

void GraphicsPipeline::Set(Ref<Buffer::Super> superBuffer)
{
    Super::Set(superBuffer);
}

void GraphicsPipeline::Set(const InputElementDescription &description)
{
    Super::Set(description);

    inputElements.resize(desc.layout.Size());
    for (size_t i = 0; i < desc.layout.Size(); i++)
    {
        inputElements[i].SemanticName         = desc.layout[i].Name().c_str();
        inputElements[i].SemanticIndex        = 0;
        inputElements[i].Format               = desc.layout[i].BaseType<DXGI_FORMAT>();
        inputElements[i].InputSlot            = 0;
        inputElements[i].AlignedByteOffset    = desc.layout[i].Offset();
		inputElements[i].InputSlotClass       = D3D11_INPUT_PER_VERTEX_DATA;
        inputElements[i].InstanceDataStepRate = 0;
    }
	stride = description.Stride;
}

void GraphicsPipeline::Create(const RenderTarget::Super *renderTarget)
{
    Reconstruct(renderTarget);
}

void GraphicsPipeline::Reconstruct(const RenderTarget::Super *superRenderTarget)
{
	if (!desc.shader)
	{
		return;
	}

	D3D11_RASTERIZER_DESC rasterizerDesc = {
		.FillMode              = D3D11_FILL_SOLID,                
	    .CullMode              = D3D11_CULL_BACK,
	    .FrontCounterClockwise = FALSE,            
	    .DepthBias             = 0,                
	    .DepthBiasClamp        = 0,    
	    .SlopeScaledDepthBias  = 0,          
	    .DepthClipEnable       = TRUE,               
	    .ScissorEnable         = FALSE,   
	    .MultisampleEnable     = FALSE,    
	    .AntialiasedLineEnable = FALSE,
    };

    device->CreateRasterizerState(&rasterizerDesc, &handle);

	Shader *shader = desc.shader.InterpretAs<Shader>();
	auto VSByteCodes = shader->GetVSByteCoeds();
	device->CreateInputLayout(inputElements.data(), U32(inputElements.size()), VSByteCodes->GetBufferPointer(), VSByteCodes->GetBufferSize(), &inputLayout);
}

void GraphicsPipeline::Draw(CommandList *cmdlist)
{
	cmdlist->IASetPrimitiveTopology(Topology);
	cmdlist->RSSetState(*this);
	cmdlist->IASetInputLayout(inputLayout.Get());
	cmdlist->VSSetShader(vertexShader.Get(), nullptr, 0);
	cmdlist->PSSetShader(pixelShader.Get(), nullptr, 0);

	UINT offset[] = {0};
	ID3D11Buffer *vertexBuffers[] = { *desc.vertexBuffers[0].As<Buffer>() };
	cmdlist->IASetVertexBuffers(0, 1, vertexBuffers, &stride, offset);
	cmdlist->IASetIndexBuffer(*desc.indexBuffer.As<Buffer>(), DXGI_FORMAT_R32_UINT, 0);

	ID3D11SamplerState *samplers[] = { *sampler };
	cmdlist->PSSetSamplers(0, 1, samplers);
	
	if (pushConstants.desc)
	{
		Bind(pushConstants.buffer, pushConstants.desc.Binding);
	}

	if (nBuffer > 0)
	{
		cmdlist->VSSetConstantBuffers(bufferStartSlot, nBuffer, buffers.data());
		cmdlist->PSSetConstantBuffers(bufferStartSlot, nBuffer, buffers.data());
	}
	if (nShaderResource > 0)
	{
		cmdlist->PSSetShaderResources(shaderResourceStartSlot, nShaderResource, (SRV **)shaderResources.data());
	}
	cmdlist->DrawIndexed(ElementCount, 0, 0);

	cmdlist->VSSetConstantBuffers(0, nBuffer, (ID3D11Buffer **)NullRef);
	cmdlist->PSSetConstantBuffers(0, nBuffer, (ID3D11Buffer **)NullRef);
	cmdlist->PSSetShaderResources(0, nShaderResource, (SRV **)NullRef);
}

void Pipeline::Bind(const DescriptorBuffer *descriptorBuffer, uint32_t binding)
{
	nShaderResource += descriptorBuffer->size();
	shaderResourceStartSlot = std::min(shaderResourceStartSlot, binding);
	memcpy(shaderResources.data(), descriptorBuffer->DeRef<uint64_t>(), sizeof(uint64_t) * descriptorBuffer->size());
}

void Pipeline::Bind(Buffer *buffer, uint32_t binding)
{
	nBuffer++;
	bufferStartSlot = std::min(bufferStartSlot, binding);
	buffers[binding] = *buffer;
}

void Pipeline::Pipeline::Bind(Buffer::Super *buffer, uint32_t binding)
{
	Bind(dynamic_cast<Buffer *>(buffer));
}

void Pipeline::Bind(Texture::Super *_texture, uint32_t binding)
{
	nShaderResource++;
	shaderResourceStartSlot  = std::min(shaderResourceStartSlot, binding);
	shaderResources[binding] = *_texture;
}

Anonymous Pipeline::AllocateDescriptorSet(uint64_t uuid)
{
	bufferStartSlot = 0;
	nBuffer = 0;

    shaderResourceStartSlot = 0;
	nShaderResource = 0;

    return Anonymize(*this);
}

void Pipeline::FreeDescriptorSet(uint64_t uuid)
{
    
}

ComputePipeline::ComputePipeline(Device *context, Shader::Super *superShader) :
    Pipeline{ context }
{
    if (!superShader)
    {
        LOG::ERR("Failed to create compute pipeline with a null shader");
        return;
    }

    auto shader = dynamic_cast<Shader *>(superShader);
	shader->QueryInterface<ID3D11ComputeShader>(&handle);

	shader->GetDesc(&pushConstants.desc);
	if (pushConstants.desc)
	{
		pushConstants.buffer = new Buffer{device, pushConstants.desc.Size, Buffer::Type::PushConstant};
	}
}

void ComputePipeline::Dispatch(uint32_t nGroupX, uint32_t nGroupY, uint32_t nGroupZ)
{
	auto context = device->GetContext();

	if (pushConstants.desc)
	{
		Pipeline::Bind(pushConstants.buffer, pushConstants.desc.Binding);
	}

	if (nBuffer > 0)
	{
		context->CSSetConstantBuffers(bufferStartSlot, nBuffer, buffers.data());
	}
	if (nShaderResource > 0)
    {
		if (pushConstants.desc)
		{
			nBuffer--;
		}
		context->CSSetUnorderedAccessViews(shaderResourceStartSlot - nBuffer , nShaderResource, (UAV **) shaderResources.data(), nullptr);
    }

	context->CSSetShader(*this, nullptr, 0);
	context->Dispatch(nGroupX, nGroupY, nGroupZ);
	context->CSSetConstantBuffers(0, nBuffer, (ID3D11Buffer **)NullRef);
	context->CSSetUnorderedAccessViews(0, nShaderResource, (UAV **)NullRef, 0);
}

void ComputePipeline::PushConstant(uint32_t size, const void *data, uint32_t offset)
{  
	if (pushConstants.buffer)
	{
		pushConstants.buffer->Update(size, data, offset);
	}
}

void ComputePipeline::Bind(SuperTexture *_texture, uint32_t binding)
{
	Texture *texture = dynamic_cast<Texture *>(_texture);
	nShaderResource++;
	shaderResourceStartSlot  = std::min(shaderResourceStartSlot, binding);
	shaderResources[binding] = texture->GetUAV();
}

}
}
