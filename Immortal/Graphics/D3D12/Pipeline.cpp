#include "Pipeline.h"
#include "Device.h"
#include "Buffer.h"
#include "Texture.h"
#include "RenderTarget.h"
#include "Instance.h"
#include "Config.h"

#if HAVE_AGILITY_SDK
#include <d3dx12/d3dx12_state_object.h>
#endif

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

static inline bool IsBlendingSupport(const DXGI_FORMAT &format)
{
    return format == DXGI_FORMAT_R8G8B8A8_UNORM     || 
           format == DXGI_FORMAT_B8G8R8A8_UNORM     || 
           format == DXGI_FORMAT_R16G16B16A16_FLOAT ||
           format == DXGI_FORMAT_R32G32B32A32_FLOAT ||
           format == DXGI_FORMAT_R10G10B10A2_UNORM;
}

Pipeline::Pipeline(Device *device, Type type) :
    NonDispatchableHandle{ device },
    type{ type }
{

}

Pipeline::~Pipeline()
{

}

static inline void MapIndex(std::vector<uint32_t> &indexMap, uint32_t baseRegister, uint32_t numRegister, uint32_t &descriptorCount)
{
	uint32_t totalRegister = baseRegister + numRegister;
	if (totalRegister >= indexMap.size())
    {
		indexMap.resize(totalRegister);
    }

    for (size_t i = baseRegister; i < totalRegister; i++)
    {
		indexMap[i] = descriptorCount++;
    }
}

void Pipeline::ConstructRootParameter(Shader *shader, std::vector<RootParameter> &rootParameters, std::vector<D3D12_DESCRIPTOR_RANGE1> &ranges, std::vector<D3D12_DESCRIPTOR_RANGE1> &samplerRanges, std::vector<D3D12_STATIC_SAMPLER_DESC> *pSamplerDesc)
{
    D3D12_SHADER_VISIBILITY visibility = shader->GetVisibility();
    auto &descriptorRanges = shader->GetDescriptorRanges();

	ranges.reserve(descriptorRanges.size() * 2);

    descriptorTables.reserve(descriptorTables.size() + descriptorRanges.size() + 1);
	rootParameters.reserve(rootParameters.size() + descriptorRanges.size() + 1);

    RootParameter rootParameter;
    auto &pushConstants = shader->GetPushConstants();
	if (pushConstants.size > 0)
    {
        if (rootParameters.empty())
        {
			hasRootConstant = true;
			rootParameter.InitAsConstants(pushConstants.size / sizeof(uint32_t), pushConstants.biding, 0, visibility);
			shaderIndexes[visibility].pushConstant = rootParameters.size();
			rootParameters.emplace_back(rootParameter);
        }
        else
        {
			rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
        }
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

		size_t j;
		for (j = 0; j < ranges.size(); j++)
		{
			if (range.RegisterSpace      == ranges[j].RegisterSpace &&
				range.BaseShaderRegister == ranges[j].BaseShaderRegister &&
				range.NumDescriptors     == ranges[j].NumDescriptors)
			{
				break;
			}
		}

		if (j == ranges.size())
		{
			MapIndex(descriptorIndexMap[range.RangeType], range.BaseShaderRegister, range.NumDescriptors, descriptorCount[heapType]);

            if (range.RangeType == D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER)
            {
				samplerRanges.emplace_back(range);
            }
            else
            {

                size_t registerSize = range.BaseShaderRegister + range.NumDescriptors;
				if (registerSize >= descriptorRangeType.size())
                {
					descriptorRangeType.resize(registerSize);
                }
				for (size_t k = range.BaseShaderRegister; k < registerSize; k++)
                {
					descriptorRangeType[k] = range.RangeType;
                }

				descriptorRangeType.emplace_back(range.RangeType);
				ranges.emplace_back(range);
            }
		}
    }
}

void Pipeline::AddDescriptorTable(std::vector<RootParameter> &rootParameters, std::vector<DescriptorTable> &descriptorTables, std::vector<D3D12_DESCRIPTOR_RANGE1> &ranges, D3D12_DESCRIPTOR_HEAP_TYPE heapType)
{
    if (ranges.empty())
    {
		return;
    }
  
	RootParameter rootParameter;
	rootParameter.InitAsDescriptorTable(UINT(ranges.size()), (DescriptorRange *)ranges.data(), D3D12_SHADER_VISIBILITY_ALL);

    DescriptorTable descriptorTable{
	    .RootParameterIndex = uint32_t(rootParameters.size()),
	    .DescriptorCount    = descriptorCount[heapType],
	    .Offset             = 0,
	    .HeapType           = heapType   
    };
	descriptorTables.emplace_back(descriptorTable);
    rootParameters.emplace_back(rootParameter);
}

void Pipeline::ConstructRootSignature(Shader **ppShader, size_t shaderCount)
{
    std::vector<D3D12_STATIC_SAMPLER_DESC> samplerDesc;
    std::vector<RootParameter> rootParameters{};

    std::vector<D3D12_DESCRIPTOR_RANGE1> ranges;
	std::vector<D3D12_DESCRIPTOR_RANGE1> samplerRanges;
    for (size_t i = 0; i < shaderCount; i++)
    {
		ConstructRootParameter(ppShader[i], rootParameters, ranges, samplerRanges, &samplerDesc);
    }

    AddDescriptorTable(rootParameters, descriptorTables, ranges,        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	AddDescriptorTable(rootParameters, descriptorTables, samplerRanges, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

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

    DX_CHECK(device->Create(signature.Get(), rootSignature->AddressOf()));
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

template <class T>
void ConstructByteCodes(T &desc, SuperShader **ppShader, size_t shaderCount)
{
	for (size_t i = 0; i < shaderCount; i++)
	{
		Shader *shader = InterpretAs<Shader>(ppShader[i]);
		auto byteCodes  = shader->GetByteCodes();
		auto visibility = shader->GetVisibility();
		if constexpr (std::is_same_v<T, PipelineMeshStateStream>)
        {
			switch (shader->GetVisibility())
			{
				case D3D12_SHADER_VISIBILITY_PIXEL:
					desc.PS = byteCodes;
					break;
				case D3D12_SHADER_VISIBILITY_MESH:
					desc.MS = byteCodes;
					break;
				case D3D12_SHADER_VISIBILITY_AMPLIFICATION:
					desc.AS = byteCodes;
					break;
				default:
					break;
			}
        }
        else
        {
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
	}
}

UINT GraphicsPipeline::ConstructRenderTargetFormats(const std::vector<Format> &outputDescription, DXGI_FORMAT *rtvFormats, DXGI_FORMAT &dsvFormat, D3D12_DEPTH_STENCIL_DESC &depthDesc, D3D12_BLEND_DESC &blendState)
{
	UINT numRenderTargets = 0;
	for (auto &format : outputDescription)
	{
		if (format.IsDepth())
		{
			dsvFormat = format;
		}
		else
		{
			rtvFormats[numRenderTargets++] = format;
		}
	}

    if (dsvFormat == DXGI_FORMAT_UNKNOWN)
    {
		depthDesc.DepthEnable = false;
    }

    if (flags & Pipeline::State::Blend)
	{
		for (size_t i = 0; i < numRenderTargets; i++)
		{
			if (!IsBlendingSupport(rtvFormats[i]))
			{
				blendState.IndependentBlendEnable = true;
				blendState.RenderTarget[i] = {};
				continue;
			}

			blendState.AlphaToCoverageEnable                 = false;
			blendState.RenderTarget[i].BlendEnable           = true;
			blendState.RenderTarget[i].SrcBlend              = D3D12_BLEND_SRC_ALPHA;
			blendState.RenderTarget[i].DestBlend             = D3D12_BLEND_INV_SRC_ALPHA;
			blendState.RenderTarget[i].BlendOp               = D3D12_BLEND_OP_ADD;
			blendState.RenderTarget[i].SrcBlendAlpha         = D3D12_BLEND_ONE;
			blendState.RenderTarget[i].DestBlendAlpha        = D3D12_BLEND_INV_SRC_ALPHA;
			blendState.RenderTarget[i].BlendOpAlpha          = D3D12_BLEND_OP_ADD;
			blendState.RenderTarget[i].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
		}
	}

    return numRenderTargets;
}

void GraphicsPipeline::ConstructGraphicsPipeline(SuperShader **ppShader, size_t shaderCount, const InputElementDescription &description, const std::vector<Format> &outputDescription)
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = ConstructDescription();
	ConstructByteCodes(desc, ppShader, shaderCount);

    D3D12_RT_FORMAT_ARRAY rtvFormats{};
	desc.NumRenderTargets = ConstructRenderTargetFormats(outputDescription, desc.RTVFormats, desc.DSVFormat, desc.DepthStencilState, desc.BlendState);

    std::vector<D3D12_INPUT_ELEMENT_DESC> inputElementDescriptions;
	SetInputElementDescription(inputElementDescriptions, description);

    desc.pRootSignature = *rootSignature;
	desc.InputLayout = {
	    .pInputElementDescs = inputElementDescriptions.data(),
	    .NumElements = uint32_t(inputElementDescriptions.size())
    };

	DX_CHECK(device->Create(&desc, &handle));
}

void GraphicsPipeline::ConstructMeshPipeline(SuperShader **ppShader, size_t shaderCount, const InputElementDescription &description, const std::vector<Format> &outputDescription)
{
	PipelineMeshStateStream desc{
	    .PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
	    .BlendState            = BlendDescription{},
	    .DepthStencilState     = DepthStencilDescription{},
	    .RasterizerState       = RasterizerDescription{},
	    .SampleDesc            = DXGI_SAMPLE_DESC{
            .Count   = 1,
            .Quality = 0
        }
    };

    desc.SampleMask = UINT_MAX;

	ConstructByteCodes(desc, ppShader, shaderCount);
	desc.RTVFormats.NumRenderTargets = ConstructRenderTargetFormats(outputDescription, desc.RTVFormats.RTFormats, desc.DSVFormat, desc.DepthStencilState, desc.BlendState);

    desc.pRootSignature = (ID3D12RootSignature *)*rootSignature;
	D3D12_PIPELINE_STATE_STREAM_DESC streamDesc{
	    .SizeInBytes                   = sizeof(desc),
	    .pPipelineStateSubobjectStream = &desc,
    };

    ComPtr<ID3D12Device2> device2;
	DX_CHECK(device->QueryInterface(device2.GetAddressOf()));
	DX_CHECK(device2->CreatePipelineState(&streamDesc, IID_PPV_ARGS(&handle)));
}

void GraphicsPipeline::Construct(SuperShader **ppShader, size_t shaderCount, const InputElementDescription &description, const std::vector<Format> &outputDescription)
{
	bool isMeshPipeline = false;
	for (size_t i = 0; i < shaderCount; i++)
	{
		Shader *shader = InterpretAs<Shader>(ppShader[i]);
		if (shader->GetVisibility() == D3D12_SHADER_VISIBILITY_MESH)
        {
			isMeshPipeline = true;
			break;
        }
    }

    ConstructRootSignature((Shader **) ppShader, shaderCount);
    if (isMeshPipeline)
    {
		ConstructMeshPipeline(ppShader, shaderCount, description, outputDescription);
    }
    else
    {
		ConstructGraphicsPipeline(ppShader, shaderCount, description, outputDescription);
    }
}

void GraphicsPipeline::SetInputElementDescription(std::vector<D3D12_INPUT_ELEMENT_DESC> &inputElementDescriptions, const InputElementDescription &description)
{
    inputElementDescriptions.resize(description.Size());
    for (size_t i = 0; i < description.Size(); i++)
    {
        inputElementDescriptions[i].SemanticName         = description[i].GetSemanticsName().c_str();
        inputElementDescriptions[i].SemanticIndex        = description[i].GetSemanticIndex();
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

    //if (!(flags & Pipeline::State::Depth))
    //{
    //    D3D12_DEPTH_STENCIL_DESC &depth = pipelineStateDesc.DepthStencilState;
    //    depth.DepthEnable             = false;
    //    depth.DepthWriteMask          = D3D12_DEPTH_WRITE_MASK_ALL;
    //    depth.DepthFunc               = D3D12_COMPARISON_FUNC_ALWAYS;
    //    depth.StencilEnable           = false;
    //    depth.FrontFace.StencilFailOp = depth.FrontFace.StencilDepthFailOp = depth.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
    //    depth.FrontFace.StencilFunc   = D3D12_COMPARISON_FUNC_ALWAYS;
    //    depth.BackFace                = depth.FrontFace;
    //}

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

    if (shader->GetStage() == ShaderStage::WorkGraph)
    {
#if HAVE_AGILITY_SDK
		CreateWorkGraph(shader);
#endif
    }
    else
    {
        D3D12_COMPUTE_PIPELINE_STATE_DESC desc = {
            .pRootSignature = *rootSignature,
            .CS             = byteCodes,
            .NodeMask       = 0,
            .CachedPSO      = {nullptr, 0},
            .Flags          = D3D12_PIPELINE_STATE_FLAG_NONE,
        };

        DX_CHECK(device->Create(&desc, &handle));
    }
}

ComputePipeline::~ComputePipeline()
{

}

#if HAVE_AGILITY_SDK
WorkGraphContext::WorkGraphContext(Device *device, ComPtr<ID3D12StateObject> stateObject, LPCWSTR pWorkGraphName)
{
	ComPtr<ID3D12StateObjectProperties1> stateObjectProperties1;
	DX_CHECK(stateObject->QueryInterface(stateObjectProperties1.GetAddressOf()));
	programIdentifier = stateObjectProperties1->GetProgramIdentifier(pWorkGraphName);

	ComPtr<ID3D12WorkGraphProperties> workgraphProperties;
	DX_CHECK(stateObject->QueryInterface(workgraphProperties.GetAddressOf()));

	UINT WorkGraphIndex = workgraphProperties->GetWorkGraphIndex(pWorkGraphName);
	workgraphProperties->GetWorkGraphMemoryRequirements(WorkGraphIndex, &memoryRequirements);

	backingMemory = new Buffer(device, BufferType::Storage, memoryRequirements.MaxSizeInBytes, MemoryType::Device);
}

WorkGraphContext::~WorkGraphContext()
{
	backingMemory.Reset();
}

void ComputePipeline::CreateWorkGraph(Shader *shader)
{
	CD3DX12_STATE_OBJECT_DESC desc(D3D12_STATE_OBJECT_TYPE_EXECUTABLE);
	auto library = desc.CreateSubobject<CD3DX12_DXIL_LIBRARY_SUBOBJECT>();

    auto byteCodes = shader->GetByteCodes();
	library->SetDXILLibrary(&byteCodes);

    ComPtr<ID3D12Device14> device14;
	DX_CHECK(device->QueryInterface(device14.GetAddressOf()));

	// DX_CHECK(device14->CreateRootSignatureFromSubobjectInLibrary(0, byteCodes.pShaderBytecode, byteCodes.BytecodeLength, L"globalRS", IID_PPV_ARGS(&rootSignature)));

    auto workgraphSubobject = desc.CreateSubobject<CD3DX12_WORK_GRAPH_SUBOBJECT>();
	workgraphSubobject->IncludeAllAvailableNodes();

    LPCWSTR workGraphName = L"HelloWorkGraphs";
	workgraphSubobject->SetProgramName(workGraphName);

    auto rootSignatureSubobject = desc.CreateSubobject<CD3DX12_GLOBAL_ROOT_SIGNATURE_SUBOBJECT>();
	rootSignatureSubobject->SetRootSignature(*rootSignature);

	DX_CHECK(device14->CreateStateObject(desc, IID_PPV_ARGS(&stateObject)));
	workgraph = new WorkGraphContext(device, stateObject, workGraphName);
}

D3D12_SET_PROGRAM_DESC ComputePipeline::GetSetProgramDesc() const
{
	D3D12_SET_PROGRAM_DESC setProgramDesc = {
	    .Type = D3D12_PROGRAM_TYPE_WORK_GRAPH,
	    .WorkGraph = {
	        .ProgramIdentifier = workgraph->programIdentifier,
	        .Flags             = D3D12_SET_WORK_GRAPH_FLAG_INITIALIZE,
	        .BackingMemory     = {
                .StartAddress = workgraph->backingMemory->GetGPUVirtualAddress(),
                .SizeInBytes  = workgraph->backingMemory->GetSize()
            }
        }
    };

    return setProgramDesc;
}

#endif

}
}
