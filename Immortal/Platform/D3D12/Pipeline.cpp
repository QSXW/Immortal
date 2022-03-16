#include "Pipeline.h"
#include "Device.h"
#include "Buffer.h"
#include "Texture.h"
#include "RenderTarget.h"

namespace Immortal
{
namespace D3D12
{

Pipeline::Pipeline(Device *device, std::shared_ptr<Shader::Super> shader) :
    Super{ shader },
    device{ device },
    state{ new State{} },
    descriptorAllocator{ DescriptorPool::Type::ShaderResourceView, DescriptorPool::Flag::ShaderVisible }
{
    descriptorAllocator.Init(device);
}

Pipeline::~Pipeline()
{
    IfNotNullThenRelease(pipelineState);
}

void Pipeline::Set(std::shared_ptr<Buffer::Super> &superBuffer)
{
    Super::Set(superBuffer);

    auto buffer = std::dynamic_pointer_cast<Buffer>(superBuffer);

    if (buffer->GetType() == Buffer::Type::Vertex)
    {
        bufferViews.vertex.BufferLocation = *buffer;
        bufferViews.vertex.SizeInBytes = buffer->Size();
    }
    else if (buffer->GetType() == Buffer::Type::Index)
    {
        bufferViews.index.BufferLocation = *buffer;
        bufferViews.index.SizeInBytes = buffer->Size();
    }
}

void Pipeline::Set(const InputElementDescription &description)
{
    Super::Set(description);
    THROWIF(!state, SError::NullPointerReference);

    auto &inputElementDesc = state->InputElementDescription;
    inputElementDesc.resize(desc.layout.Size());
    for (size_t i = 0; i < desc.layout.Size(); i++)
    {
        inputElementDesc[i].SemanticName         = desc.layout[i].Name().c_str();
        inputElementDesc[i].SemanticIndex        = 0;
        inputElementDesc[i].Format               = desc.layout[i].BaseType<DXGI_FORMAT>();
        inputElementDesc[i].InputSlot            = 0;
        inputElementDesc[i].AlignedByteOffset    = desc.layout[i].Offset();
        inputElementDesc[i].InputSlotClass       = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
        inputElementDesc[i].InstanceDataStepRate = 0;
    }
    bufferViews.vertex.StrideInBytes = desc.layout.Stride;
}

void Pipeline::Create(const std::shared_ptr<RenderTarget::Super> &renderTarget, Option option)
{
    Reconstruct(renderTarget);
}

void Pipeline::Reconstruct(const std::shared_ptr<RenderTarget::Super> &superRenderTarget, Option option)
{
    auto shader = std::dynamic_pointer_cast<Shader>(desc.shader);
    if (!shader)
    {
        return;
    }

    auto &bytesCodes       = shader->ByteCodes();
    auto &descriptorRanges = shader->DescriptorRanges();

    std::vector<D3D12_STATIC_SAMPLER_DESC> samplers;
    std::vector<RootParameter> rootParameters{};

    descriptorTables.resize(descriptorRanges.size());
    rootParameters.resize(descriptorRanges.size());
    uint32_t offset = 0;
    for (size_t i = 0; i < descriptorRanges.size(); i++)
    {
        auto &range = descriptorRanges[i].first;
        if (range.RangeType == D3D12_DESCRIPTOR_RANGE_TYPE_SRV)
        {
            if (samplers.empty())
            {
                D3D12_STATIC_SAMPLER_DESC sampler = {};
                sampler.Filter           = D3D12_FILTER_MIN_MAG_MIP_POINT;
                sampler.AddressU         = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
                sampler.AddressV         = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
                sampler.AddressW         = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
                sampler.MipLODBias       = 0;
                sampler.MaxAnisotropy    = 0;
                sampler.ComparisonFunc   = D3D12_COMPARISON_FUNC_NEVER;
                sampler.BorderColor      = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
                sampler.MinLOD           = 0.0f;
                sampler.MaxLOD           = D3D12_FLOAT32_MAX;
                sampler.ShaderRegister   = 0;
                sampler.RegisterSpace    = 0;
                sampler.ShaderVisibility = descriptorRanges[i].second;

                samplers.emplace_back(std::move(sampler));
            }
        }

        RootParameter rootParameter;
        rootParameter.InitAsDescriptorTable(1, &range, descriptorRanges[i].second);
        rootParameters[i] = std::move(rootParameter);
        descriptorTables[i] = DescriptorTable{ range.NumDescriptors, offset };
        offset += range.NumDescriptors;
    }

    RootSignature::Description rootSignatureDesc{
        U32(rootParameters.size()),
        rootParameters.data(),
        U32(samplers.size()),
        samplers.data(),
        RootSignature::Flag::AllowInputAssemblerInputLayout
    };

    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> error;
    if (FAILED(D3D12SerializeVersionedRootSignature(&rootSignatureDesc, &signature, &error)) && error)
    {
        const char *msg = rcast<const char *>(error->GetBufferPointer());
        LOG::ERR("{0}", msg);
        THROWIF(true, msg);
    }

    device->Create(0, signature.Get(), &rootSignature);

    D3D12_INPUT_LAYOUT_DESC inputLayoutDesc = { 
        state->InputElementDescription.data(),
        U32(state->InputElementDescription.size())
    };

    D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineStateDesc{};
    CleanUpObject(&pipelineStateDesc);
    pipelineStateDesc.InputLayout                     = inputLayoutDesc;
    pipelineStateDesc.pRootSignature                  = rootSignature;
    pipelineStateDesc.VS                              = bytesCodes[Shader::VertexShaderPos];
    pipelineStateDesc.PS                              = bytesCodes[Shader::PixelShaderPos ];
    pipelineStateDesc.RasterizerState                 = RasterizerDescription{};
    pipelineStateDesc.BlendState                      = BlendDescription{};
    pipelineStateDesc.DepthStencilState               = DepthStencilDescription{};
    pipelineStateDesc.SampleMask                      = UINT_MAX;
    pipelineStateDesc.PrimitiveTopologyType           = SuperToBase(desc.PrimitiveType);
    pipelineStateDesc.NumRenderTargets                = 1;
    pipelineStateDesc.SampleDesc.Count                = 1;

    auto renderTarget = std::dynamic_pointer_cast<RenderTarget>(superRenderTarget);
    auto &colorBuffers = renderTarget->GetColorBuffers();

    THROWIF(colorBuffers.size() > SL_ARRAY_LENGTH(pipelineStateDesc.RTVFormats), SError::OutOfBound);
    pipelineStateDesc.NumRenderTargets = colorBuffers.size();
    for (int i = 0; i < pipelineStateDesc.NumRenderTargets; i++)
    {
        pipelineStateDesc.RTVFormats[i] = colorBuffers[i].Format;
    }

    auto &depthBuffer = renderTarget->GetDepthBuffer();
    pipelineStateDesc.DSVFormat = depthBuffer.Format;

    device->Create(&pipelineStateDesc, &pipelineState);
}

void Pipeline::Bind(const std::string &name, const Buffer::Super *super)
{
    (void)name;

    auto buffer = RemoveConst(dcast<const Buffer *>(super));
    DescriptorTable &descriptorTable = descriptorTables[buffer->Binding()];

    auto cbvDescriptor  = descriptorAllocator.Bind(device, descriptorTable.Offset);
    device->CopyDescriptors(
        1, cbvDescriptor.cpu,
        buffer->GetDescriptor(),
        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
    );
}

void Pipeline::Bind(const Descriptor::Super *super, uint32_t binding)
{
    auto descriptors = rcast<const CPUDescriptor *>(super);

    DescriptorTable &descriptorTable = descriptorTables[binding];
    auto srvDescriptor = descriptorAllocator.Bind(device, descriptorTable.Offset);
    for (size_t i = 0; i < descriptorTable.DescriptorCount; i++)
    {
        device->CopyDescriptors(
            1, srvDescriptor.cpu,
            descriptors[i],
            D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
        );
        srvDescriptor.Offset(1, descriptorAllocator.Increment());
    }
}

void Pipeline::Bind(Texture::Super *super, uint32_t binding)
{
    DescriptorTable &descriptorTable = descriptorTables[binding];
    Texture *texture = dcast<Texture *>(super);
    auto srvDescriptor = descriptorAllocator.Bind(device, descriptorTable.Offset);
    device->CopyDescriptors(
        1, srvDescriptor.cpu,
        texture->GetDescriptor(),
        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
    );
}

}
}
