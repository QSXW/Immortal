#pragma once

#include "Common.h"
#include "Algorithm/LightArray.h"
#include "Render/RenderTarget.h"
#include "Render/Pipeline.h"
#include "Buffer.h"
#include "Shader.h"
#include "Texture.h"
#include "Sampler.h"

namespace Immortal
{
namespace D3D11
{

class Device;
class Pipeline : public virtual SuperPipeline
{
public:
    using Super = SuperPipeline;

public:
    Pipeline(Device *device);

    virtual ~Pipeline();

    virtual void Bind(const DescriptorBuffer *descriptors, uint32_t binding = 0) override;

    virtual void Bind(SuperBuffer *buffer, uint32_t binding = 0) override;

    virtual void Bind(SuperTexture *texture, uint32_t binding = 0) override;

    virtual Anonymous AllocateDescriptorSet(uint64_t uuid) override;

    virtual void FreeDescriptorSet(uint64_t uuid) override;

    void Bind(Buffer *buffer, uint32_t binding = 0);

protected:
	Device *device;

    uint32_t bufferStartSlot = 0;
	uint32_t nBuffer = 0;
    std::array<ID3D11Buffer*, 16> buffers;

    uint32_t shaderResourceStartSlot = 0;
	uint32_t nShaderResource = 0;
    std::array<uint64_t, 32> shaderResources;

    struct
    {
		D3D_PUSH_CONSTANT_DESC desc;
		URef<Buffer> buffer;
    } pushConstants;
};

class GraphicsPipeline : public Pipeline, public SuperGraphicsPipeline
{
public:
    using Super = SuperGraphicsPipeline;

    using Primitive = ID3D11RasterizerState;
	D3D11_OPERATOR_HANDLE()

public:
    GraphicsPipeline(Device *device, Ref<Shader::Super> shader);

    GraphicsPipeline(Device *device, Ref<Shader> shader);

    virtual void Create(const SuperRenderTarget *renderTarget) override;

    virtual void Reconstruct(const SuperRenderTarget *renderTarget) override;

    virtual void Set(Ref<Buffer::Super> buffer) override;

    virtual void Set(const InputElementDescription &description) override;

    void Draw(CommandList *cmdlist);

public:
   template <Buffer::Type type>
    Buffer *GetBuffer()
    {
        if constexpr (type == Buffer::Type::Vertex)
        {
            return dynamic_cast<Buffer*>(desc.vertexBuffers[0].Get());
        }
        if constexpr (type == Buffer::Type::Index)
        {
            return dynamic_cast<Buffer*>(desc.indexBuffer.Get());
        }
    }

public:
	D3D11_PRIMITIVE_TOPOLOGY Topology;

protected:
	ComPtr<ID3D11InputLayout> inputLayout;

    ComPtr<ID3D11RasterizerState> rasterizerState;

    ComPtr<ID3D11VertexShader> vertexShader;

	ComPtr<ID3D11PixelShader> pixelShader;

    URef<Sampler> sampler;

    std::vector<D3D11_INPUT_ELEMENT_DESC> inputElements;

    UINT stride = 0;
};

class ComputePipeline : public Pipeline, public SuperComputePipeline
{
public:
    using Super = SuperComputePipeline;

    using Primitive = ID3D11ComputeShader;
	D3D11_OPERATOR_HANDLE()

public:
    ComputePipeline(Device *device, Shader::Super *shader);

    void Dispatch(uint32_t nGroupX, uint32_t nGroupY, uint32_t nGroupZ = 0);

    void PushConstant(uint32_t size, const void *data, uint32_t offset = 0);

    virtual void Bind(SuperTexture *texture, uint32_t binding = 0) override;
};

}
}
