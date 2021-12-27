#include "Pipeline.h"
#include "Device.h"
#include "Buffer.h"

#include "RenderTarget.h"
namespace Immortal
{
namespace D3D12
{

Pipeline::Pipeline(Device *device, std::shared_ptr<Shader::Super> shader) :
    Super{ shader },
    device{ device },
    state{ new State{} }
{

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
    ThrowIf(!state, SError::NullPointerReference);

    auto &inputElementDesc = state->InputElementDescription;
    inputElementDesc.resize(description.Size());
    for (size_t i = 0; i < description.Size(); i++)
    {
        inputElementDesc[i].SemanticName         = description[i].Name().c_str();
        inputElementDesc[i].SemanticIndex        = 0;
        inputElementDesc[i].Format               = description[i].BaseType<DXGI_FORMAT>();
        inputElementDesc[i].InputSlot            = 0;
        inputElementDesc[i].AlignedByteOffset    = description[i].Offset();
        inputElementDesc[i].InputSlotClass       = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
        inputElementDesc[i].InstanceDataStepRate = 0;
    }
    bufferViews.vertex.StrideInBytes = description.Stride();
}

void Pipeline::Create(const std::shared_ptr<RenderTarget::Super> &renderTarget)
{
    // Reconstruct(renderTarget);
}

void Pipeline::Reconstruct(const std::shared_ptr<RenderTarget::Super> &superRenderTarget)
{
    auto shader = std::dynamic_pointer_cast<Shader>(desc.shader);
    auto &bytesCodes       = shader->ByteCodes();
    auto &descriptorRanges = shader->DescriptorRanges();

    std::vector<RootParameter> rootParameters{};
    for (auto &range : descriptorRanges)
    {
        RootParameter rootParameter;
        rootParameter.InitAsDescriptorTable(range.NumDescriptors, &range, D3D12_SHADER_VISIBILITY_VERTEX);
        rootParameters.emplace_back(std::move(rootParameter));
    }

    RootSignature::Description rootSignatureDesc{
        U32(rootParameters.size()),
        rootParameters.data(),
        0,
        nullptr,
        RootSignature::Flag::AllowInputAssemblerInputLayout
    };

    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> error;
    Check(D3D12SerializeVersionedRootSignature(&rootSignatureDesc, &signature, &error));
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
    pipelineStateDesc.DepthStencilState.DepthEnable   = FALSE;
    pipelineStateDesc.DepthStencilState.StencilEnable = FALSE;
    pipelineStateDesc.SampleMask                      = UINT_MAX;
    pipelineStateDesc.PrimitiveTopologyType           = SuperToBase(desc.PrimitiveType);
    pipelineStateDesc.NumRenderTargets                = 1;
    pipelineStateDesc.SampleDesc.Count                = 1;

    auto renderTarget = std::dynamic_pointer_cast<RenderTarget>(superRenderTarget);
    auto &colorBuffers = renderTarget->GetColorBuffers();

    ThrowIf(colorBuffers.size() > SL_ARRAY_LENGTH(pipelineStateDesc.RTVFormats), SError::OutOfBound);
    pipelineStateDesc.NumRenderTargets = colorBuffers.size();
    for (int i = 0; i < pipelineStateDesc.NumRenderTargets; i++)
    {
        pipelineStateDesc.RTVFormats[i] = colorBuffers[i].Format;
    }

    auto &depthBuffer = renderTarget->GetDepthBuffer();
    pipelineStateDesc.DSVFormat = depthBuffer.Format;

    device->Create(&pipelineStateDesc, &pipelineState);
}

}
}
