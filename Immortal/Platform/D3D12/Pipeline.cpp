#include "Pipeline.h"
#include "Device.h"
#include "Buffer.h"
#include "Texture.h"
#include "RenderTarget.h"
#include "RenderContext.h"

namespace Immortal
{
namespace D3D12
{

static D3D12_PRIMITIVE_TOPOLOGY_TYPE ConvertPrimitiveTopologyType(const Pipeline::PrimitiveType type)
{
    switch (type)
    {
    case Pipeline::PrimitiveType::Line:
        return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;

    case Pipeline::PrimitiveType::Point:
        return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;

    case Pipeline::PrimitiveType::Triangles:
    default:
        return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    }
}

Pipeline::Pipeline(RenderContext *context) :
    context{ context }
{

}

Pipeline::~Pipeline()
{

}

void Pipeline::InitRootSignature(const Shader *shader)
{
    rootSignature = new RootSignature;

    auto &descriptorRanges = shader->DescriptorRanges();

    std::vector<D3D12_STATIC_SAMPLER_DESC> samplers;
    std::vector<RootParameter> rootParameters{};

    descriptorTables.resize(descriptorRanges.size());
    rootParameters.resize(descriptorRanges.size() + 1);
    rootParameters[0].InitAsConstants(Limit::BytesOfRootConstant / sizeof(uint32_t), Definitions::RootConstantsIndex);

    uint32_t offset = 0;
    for (size_t i = 0; i < descriptorRanges.size(); i++)
    {
        auto &descriptorRange = descriptorRanges[i];
        auto &range = descriptorRange.first;
        if (range.RangeType == D3D12_DESCRIPTOR_RANGE_TYPE_SRV || range.RangeType == D3D12_DESCRIPTOR_RANGE_TYPE_UAV)
        {
            textureIndexInDescriptorTable = i;
            if (samplers.empty())
            {
                D3D12_STATIC_SAMPLER_DESC sampler = {
                    .Filter           = D3D12_FILTER_MIN_MAG_MIP_POINT,
                    .AddressU         = D3D12_TEXTURE_ADDRESS_MODE_BORDER,
                    .AddressV         = D3D12_TEXTURE_ADDRESS_MODE_BORDER,
                    .AddressW         = D3D12_TEXTURE_ADDRESS_MODE_BORDER,
                    .MipLODBias       = 0,
                    .MaxAnisotropy    = 0,
                    .ComparisonFunc   = D3D12_COMPARISON_FUNC_NEVER,
                    .BorderColor      = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK,
                    .MinLOD           = 0.0f,
                    .MaxLOD           = D3D12_FLOAT32_MAX,
                    .ShaderRegister   = 0,
                    .RegisterSpace    = 0,
                    .ShaderVisibility = descriptorRanges[i].second,
                };
                samplers.emplace_back(std::move(sampler));
            }
        }

        rootParameters[i + 1].InitAsDescriptorTable(1, &range, descriptorRange.second);
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

    Device *device = context->GetAddress<Device>();
    Check(device->Create(signature.Get(), &*rootSignature));
#ifdef _DEBUG
    rootSignature->SetName("Pipeline::RootSignature");
#endif
}

GraphicsPipeline::GraphicsPipeline(RenderContext *context, Ref<Shader::Super> shader) :
    Super{ shader },
    Pipeline{ context },
    state{ new State{} }
{

}

void GraphicsPipeline::Set(Ref<Buffer::Super> superBuffer)
{
    Super::Set(superBuffer);

    auto buffer = dynamic_cast<Buffer *>(superBuffer.Get());

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

void GraphicsPipeline::Set(const InputElementDescription &description)
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

void GraphicsPipeline::Create(const RenderTarget::Super *renderTarget)
{
    Reconstruct(renderTarget);
}

void GraphicsPipeline::Reconstruct(const RenderTarget::Super *superRenderTarget)
{
    auto shader = desc.shader.InterpretAs<Shader>();
    if (!shader)
    {
        return;
    }

    auto &bytesCodes = shader->ByteCodes();
    InitRootSignature(shader);

    D3D12_INPUT_LAYOUT_DESC inputLayoutDesc = { 
        state->InputElementDescription.data(),
        U32(state->InputElementDescription.size())
    };

    auto renderTarget = dynamic_cast<const RenderTarget*>(superRenderTarget);
    auto &depthBuffer = renderTarget->GetDepthBuffer();

    D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineStateDesc = {
        .pRootSignature        = *rootSignature,
        .VS                    = bytesCodes[Shader::VertexShaderPos],
        .PS                    = bytesCodes[Shader::PixelShaderPos ],
        .DS                    = nullptr,
        .HS                    = nullptr,
        .GS                    = nullptr,
        .StreamOutput          = { 
            .pSODeclaration   = nullptr,
            .NumEntries       = 0,
            .pBufferStrides   = nullptr,
            .NumStrides       = 0,
            .RasterizedStream = 0,
         },
        .BlendState            = BlendDescription{},
        .SampleMask            = UINT_MAX,
        .RasterizerState       = RasterizerDescription{},
        .DepthStencilState     = DepthStencilDescription{},
        .InputLayout           = inputLayoutDesc,
        .IBStripCutValue       = { D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED },
        .PrimitiveTopologyType = ConvertPrimitiveTopologyType(desc.primitiveType),
        .NumRenderTargets      = 1,
        .RTVFormats            = {},
        .DSVFormat             = depthBuffer.Format,
        .SampleDesc            = { .Count = 1, .Quality = 0 },
        .NodeMask              = 0,
        .CachedPSO             = { .pCachedBlob = nullptr, .CachedBlobSizeInBytes = 0 },
        .Flags                 = D3D12_PIPELINE_STATE_FLAG_NONE,
    };

    auto &colorBuffers = renderTarget->GetColorBuffers();
    THROWIF(colorBuffers.size() > SL_ARRAY_LENGTH(pipelineStateDesc.RTVFormats), SError::OutOfBound);
    pipelineStateDesc.NumRenderTargets = colorBuffers.size();
    for (int i = 0; i < pipelineStateDesc.NumRenderTargets; i++)
    {
        pipelineStateDesc.RTVFormats[i] = colorBuffers[i].Format;
    }

    Device *device = context->GetAddress<Device>();
    device->Create(&pipelineStateDesc, &handle);
}

void Pipeline::Bind(const DescriptorBuffer *descriptorBuffer, uint32_t binding)
{
    Bind(descriptorBuffer->DeRef<CPUDescriptor>(), binding);
}

void Pipeline::Bind(Buffer *buffer, uint32_t binding)
{
    Device *device = context->GetAddress<Device>();
    DescriptorTable &descriptorTable = descriptorTables[Definitions::ConstantBufferIndex];

    auto cbvDescriptor = descriptorHeap.active->StartOfCPU();
    cbvDescriptor.Offset(descriptorTable.Offset + binding, descriptorHeap.active->GetIncrement());
    device->CopyDescriptors(
        1, cbvDescriptor,
        buffer->GetDescriptor(),
        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
    );
}

void Pipeline::Pipeline::Bind(Buffer::Super *super, uint32_t binding)
{
    auto buffer = dcast<Buffer *>(super);
    Bind(buffer, binding);
}

void Pipeline::Bind(const std::string &name, const Buffer::Super *super)
{
    (void)name;

    auto buffer = dcast<Buffer *>(ccast<Buffer::Super *>(super));
    Bind(buffer, buffer->Binding());
}

void Pipeline::Bind(const CPUDescriptor *descriptors, uint32_t binding)
{
    Device *device = context->GetAddress<Device>();
    auto increment = descriptorHeap.active->GetIncrement();

    DescriptorTable &descriptorTable = descriptorTables[textureIndexInDescriptorTable];
    auto srvDescriptor = descriptorHeap.active->StartOfCPU();
    srvDescriptor.Offset(descriptorTable.Offset, increment);
    for (size_t i = 0; i < descriptorTable.DescriptorCount; i++)
    {
        device->CopyDescriptors(
            1, srvDescriptor,
            descriptors[i],
            D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
        );
        srvDescriptor.Offset(1, increment);
    }
}

void Pipeline::Bind(Texture::Super *super, uint32_t binding)
{
    Device *device = context->GetAddress<Device>();
    DescriptorTable &descriptorTable = descriptorTables[textureIndexInDescriptorTable];
    Texture *texture = dcast<Texture *>(super);
    auto srvDescriptor = descriptorHeap.active->StartOfCPU();
    srvDescriptor.Offset(descriptorTable.Offset + binding, descriptorHeap.active->GetIncrement());
    device->CopyDescriptors(
        1, srvDescriptor,
        texture->GetDescriptor(),
        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
    );
}

Anonymous Pipeline::AllocateDescriptorSet(uint64_t uuid)
{
    Device *device = context->GetAddress<Device>();
    auto it = descriptorHeap.packs.find(uuid);
    if (it != descriptorHeap.packs.end())
    {
        descriptorHeap.active = it->second;
    }
    else
    {
        if (!descriptorHeap.freePacks.empty())
        {
            descriptorHeap.active = descriptorHeap.freePacks.front();
            descriptorHeap.freePacks.pop();
        }
        else
        {
            auto &descriptorTable = descriptorTables.back();
            descriptorHeap.active = new DescriptorHeap{
                device,
                (descriptorTable.Offset + descriptorTable.DescriptorCount) * 3,
                DescriptorHeap::Type::ShaderResourceView,
                DescriptorHeap::Flag::ShaderVisible
            };
            descriptorHeap.packs[uuid] = descriptorHeap.active;
        }
    }

    return Anonymize(descriptorHeap.active);
}

void Pipeline::FreeDescriptorSet(uint64_t uuid)
{
    auto it = descriptorHeap.packs.find(uuid);
    if (it != descriptorHeap.packs.end())
    {
        descriptorHeap.freePacks.push(it->second);
        descriptorHeap.packs.erase(it);
    }
}

ComputePipeline::ComputePipeline(RenderContext *context, Shader::Super *superShader) :
    Pipeline{ context }
{
    Device *device = context->GetAddress<Device>();

    if (!superShader)
    {
        LOG::ERR("Failed to create compute pipeline with a null shader");
        return;
    }
    auto shader = dynamic_cast<Shader *>(superShader);
    auto &byteCodes = shader->ByteCodes();

    InitRootSignature(shader);

    D3D12_COMPUTE_PIPELINE_STATE_DESC desc = {
        .pRootSignature = *rootSignature,
        .CS             = byteCodes[Shader::ComputeShaderPos],
        .NodeMask       = 0,
        .CachedPSO      = {nullptr, 0},
        .Flags          = D3D12_PIPELINE_STATE_FLAG_NONE,
    };

    Check(device->Create(&desc, &handle));
}

void ComputePipeline::Dispatch(uint32_t nGroupX, uint32_t nGroupY, uint32_t nGroupZ)
{
    context->Compute([&, this](CommandList *cmdlist) {
        cmdlist->SetPipelineState(*this);
        cmdlist->SetComputeRootSignature(*rootSignature);
        cmdlist->SetDescriptorHeaps(&*descriptorHeap.active, 1);

        GPUDescriptor descriptors{ descriptorHeap.active->StartOfGPU() };
        for (uint32_t i = 0; i < descriptorTables.size(); i++)
        {
            GPUDescriptor descriptor = descriptors;
            descriptor.Offset(descriptorTables[i].Offset, descriptorHeap.active->GetIncrement());
            cmdlist->SetComputeRootDescriptorTable(i + 1 /* 0 is reserved by root constants */, descriptor);
        }

        cmdlist->PushComputeConstant(pushConstants.size(), pushConstants.data(), 0);
        cmdlist->Dispatch(nGroupX, nGroupY, nGroupZ);
        });

    pushConstants.clear();
}

void ComputePipeline::PushConstant(uint32_t size, const void *data, uint32_t offset)
{  
    pushConstants.resize(size + offset);
    memcpy(pushConstants.data() + offset, data, size);
}

}
}
