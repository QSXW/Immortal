#include "Pipeline.h"
#include "Device.h"
#include "Buffer.h"
#include "Texture.h"
#include "RenderTarget.h"
#include "Instance.h"

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

Pipeline::Pipeline(Device *device, Type type) :
    NonDispatchableHandle{ device },
    type{ type }
{

}

Pipeline::~Pipeline()
{

}

void Pipeline::ConstructRootParameter(Shader *shader, std::vector<RootParameter> *pRootParameters, std::vector<D3D12_STATIC_SAMPLER_DESC> *pSamplerDesc)
{
    D3D12_SHADER_VISIBILITY visibility = shader->GetVisibility();
    auto &descriptorRanges = shader->GetDescriptorRanges();

    descriptorTables.reserve(descriptorTables.size() + descriptorRanges.size() + 1);
    pRootParameters->reserve(pRootParameters->size() + descriptorRanges.size() + 1);

    RootParameter rootParameter;
    auto &pushConstants = shader->GetPushConstants();
    if (pushConstants.size > 0)
    {
        hasRootConstant = true;
        rootParameter.InitAsConstants(pushConstants.size / sizeof(uint32_t), pushConstants.biding, 0, visibility);
        shaderIndexes[visibility].pushConstant = pRootParameters->size();
        pRootParameters->emplace_back(rootParameter);
    }

    uint32_t offsets[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES] = {};
    for (size_t i = 0; i < descriptorRanges.size(); i++)
    {
        auto &range = descriptorRanges[i];
        D3D12_DESCRIPTOR_HEAP_TYPE heapType = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        if (range.RangeType == D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER)
        {
            heapType = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
        }

		descriptorIndexMap[heapType].resize(range.BaseShaderRegister + 1);
		descriptorIndexMap[heapType][range.BaseShaderRegister] = descriptorCount[heapType]++;

        auto &offset = offsets[heapType];
        rootParameter.InitAsDescriptorTable(1, &range, visibility);
        descriptorTables.emplace_back(DescriptorTable{
		    .RootParameterIndex = uint32_t(pRootParameters->size()),
            .DescriptorCount    = range.NumDescriptors,
            .Offset             = offset,
            .HeapType           = heapType
        });

        pRootParameters->emplace_back(rootParameter);
        offset += range.NumDescriptors;
    }
}

void Pipeline::ConstructRootSignature(Shader **ppShader, size_t shaderCount)
{
    std::vector<D3D12_STATIC_SAMPLER_DESC> samplerDesc;
    std::vector<RootParameter> rootParameters{};

    for (size_t i = 0; i < shaderCount; i++)
    {
        ConstructRootParameter(ppShader[i], &rootParameters, &samplerDesc);
    }

    RootSignature::Description rootSignatureDesc{
	    uint32_t(rootParameters.size()),
        rootParameters.data(),
	    uint32_t(samplerDesc.size()),
        samplerDesc.data(),
        RootSignature::Flag::AllowInputAssemblerInputLayout};

    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> error;

    rootSignature = new RootSignature;
	if (FAILED(Instance::SerializeVersionedRootSignature(&rootSignatureDesc, &signature, &error)) && error)
    {
        const char *msg = (const char *)(error->GetBufferPointer());
        LOG::ERR("{0}", msg);
        THROWIF(true, msg);
    }

    Check(device->Create(signature.Get(), rootSignature->AddressOf()));
#ifdef _DEBUG
    rootSignature->SetName("Pipeline::RootSignature");
#endif
}

GraphicsPipeline::GraphicsPipeline(Device *device) :
    Pipeline{ device }
{

}

GraphicsPipeline::~GraphicsPipeline()
{

}

void GraphicsPipeline::Construct(SuperShader **ppShader, size_t shaderCount, const InputElementDescription &description, const std::vector<Format> &outputDescription)
{
    ConstructRootSignature((Shader **) ppShader, shaderCount);

    auto desc = ConstructDescription();
    for (size_t i = 0; i < shaderCount; i++)
    {
        Shader *shader = InterpretAs<Shader>(ppShader[i]);
        auto byteCodes = shader->GetByteCodes();
        switch (shader->GetVisibility())
        {
            case D3D12_SHADER_VISIBILITY_VERTEX:
                desc.VS = byteCodes;
                break;
            case D3D12_SHADER_VISIBILITY_PIXEL:
                desc.PS = byteCodes;
                break;
            case D3D12_SHADER_VISIBILITY_GEOMETRY:
                desc.GS = byteCodes;
                break;
            case D3D12_SHADER_VISIBILITY_DOMAIN:
                desc.DS = byteCodes;
                break;
            case D3D12_SHADER_VISIBILITY_HULL:
                desc.HS = byteCodes;
                break;
            default:
                break;
        }
    }

    desc.NumRenderTargets = 0;
    for (auto &format : outputDescription)
    {
        if (format.IsDepth())
        {
            desc.DSVFormat = format;
        }
        else
        {
            desc.RTVFormats[desc.NumRenderTargets++] = format;
        }
    }

    if (desc.DSVFormat == DXGI_FORMAT_UNKNOWN)
    {
		desc.DepthStencilState.DepthEnable = false;
    }

    std::vector<D3D12_INPUT_ELEMENT_DESC> inputElementDescriptions;
    SetInputElementDescription(inputElementDescriptions, description);

    desc.pRootSignature = *rootSignature;
    desc.InputLayout = {
        .pInputElementDescs = inputElementDescriptions.data(),
        .NumElements = uint32_t(inputElementDescriptions.size())
    };

    Check(device->Create(&desc, &handle));
}

void GraphicsPipeline::SetInputElementDescription(std::vector<D3D12_INPUT_ELEMENT_DESC> &inputElementDescriptions, const InputElementDescription &description)
{
    inputElementDescriptions.resize(description.Size());
    for (size_t i = 0; i < description.Size(); i++)
    {
        inputElementDescriptions[i].SemanticName         = description[i].GetSemanticsName().c_str();
        inputElementDescriptions[i].SemanticIndex        = 0;
        inputElementDescriptions[i].Format               = description[i].GetFormat();
        inputElementDescriptions[i].InputSlot            = 0;
        inputElementDescriptions[i].AlignedByteOffset    = description[i].GetOffset();
        inputElementDescriptions[i].InputSlotClass       = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
        inputElementDescriptions[i].InstanceDataStepRate = 0;
    }
    vertexEntryStride = description.GetStride();
}

D3D12_GRAPHICS_PIPELINE_STATE_DESC GraphicsPipeline::ConstructDescription()
{
    D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineStateDesc = {
        .pRootSignature        = nullptr,
        .VS                    = nullptr,
        .PS                    = nullptr,
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
        .InputLayout           = {},
        .IBStripCutValue       = { D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED },
	    .PrimitiveTopologyType = ConvertPrimitiveTopologyType(PrimitiveType::Triangles),
        .NumRenderTargets      = 1,
        .RTVFormats            = {},
        .DSVFormat             = DXGI_FORMAT_UNKNOWN,
        .SampleDesc            = { .Count = 1, .Quality = 0 },
        .NodeMask              = 0,
        .CachedPSO             = { .pCachedBlob = nullptr, .CachedBlobSizeInBytes = 0 },
        .Flags                 = D3D12_PIPELINE_STATE_FLAG_NONE,
    };

    if (!(flags & Pipeline::State::Depth))
    {
        D3D12_DEPTH_STENCIL_DESC &depth = pipelineStateDesc.DepthStencilState;
        depth.DepthEnable             = false;
        depth.DepthWriteMask          = D3D12_DEPTH_WRITE_MASK_ALL;
        depth.DepthFunc               = D3D12_COMPARISON_FUNC_ALWAYS;
        depth.StencilEnable           = false;
        depth.FrontFace.StencilFailOp = depth.FrontFace.StencilDepthFailOp = depth.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
        depth.FrontFace.StencilFunc   = D3D12_COMPARISON_FUNC_ALWAYS;
        depth.BackFace                = depth.FrontFace;
    }

    if (flags & Pipeline::State::Blend)
    {
        auto &blend = pipelineStateDesc.BlendState;
        blend.AlphaToCoverageEnable                 = false;
        blend.RenderTarget[0].BlendEnable           = true;
        blend.RenderTarget[0].SrcBlend              = D3D12_BLEND_SRC_ALPHA;
        blend.RenderTarget[0].DestBlend             = D3D12_BLEND_INV_SRC_ALPHA;
        blend.RenderTarget[0].BlendOp               = D3D12_BLEND_OP_ADD;
        blend.RenderTarget[0].SrcBlendAlpha         = D3D12_BLEND_ONE;
        blend.RenderTarget[0].DestBlendAlpha        = D3D12_BLEND_INV_SRC_ALPHA;
        blend.RenderTarget[0].BlendOpAlpha          = D3D12_BLEND_OP_ADD;
        blend.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    }

    return pipelineStateDesc;
}

ComputePipeline::ComputePipeline(Device *device, SuperShader *shader) :
    ComputePipeline{ device, InterpretAs<Shader>(shader) }
{

}

ComputePipeline::ComputePipeline(Device *device, Shader *shader) :
    Pipeline{ device, Type::Compute }
{
    auto byteCodes = shader->GetByteCodes();

    ConstructRootSignature(&shader, 1);

    D3D12_COMPUTE_PIPELINE_STATE_DESC desc = {
        .pRootSignature = *rootSignature,
        .CS             = byteCodes,
        .NodeMask       = 0,
        .CachedPSO      = {nullptr, 0},
        .Flags          = D3D12_PIPELINE_STATE_FLAG_NONE,
    };

    Check(device->Create(&desc, &handle));
}

ComputePipeline::~ComputePipeline()
{

}

}
}
