#include "DirectXShaderCompiler.h"

#include "Core.h"
#include "Shared/DLLLoader.h"

#include <dxcapi.h>
#include <wrl/client.h>
using Microsoft::WRL::ComPtr;

namespace Immortal
{

static inline const wchar_t *GetShaderTarget(ShaderStage stage)
{
    switch (stage)
    {
        case ShaderStage::Vertex:
            return L"vs_6_1";
        case ShaderStage::Pixel:
            return L"ps_6_1";
        case ShaderStage::Compute:
            return L"cs_6_1";
        default:
            return nullptr;
    }
}

DirectXShaderCompiler::DirectXShaderCompiler() :
    dxc{},
    utils{},
    compiler{}
{
    dxc = { "dxcompiler.dll" };
    auto pDxcCreateInstance = dxc.GetFunc<decltype(&DxcCreateInstance)>("DxcCreateInstance");
    if (!pDxcCreateInstance)
    {
		LOG::ERR("Failed to get handle to DxcCreateInstance!");
		return;
    }
    if (FAILED(pDxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler))))
    {
		LOG::ERR("Failed to init DirectX Shader Compiler");
		return;
    }

    if (FAILED(pDxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils))))
    {
	   LOG::ERR("Could not init DXC Utiliy");
        return;
    }
}

DirectXShaderCompiler::~DirectXShaderCompiler()
{
    if (compiler)
    {
		compiler->Release();
		compiler = nullptr;
    }
    if (utils)
    {
		utils->Release();
		compiler = nullptr;
    }
}

bool DirectXShaderCompiler::Compile(const std::string &name, ShaderSourceType sourceType, ShaderBinaryType binaryType, ShaderStage stage, uint32_t size, const char *data, const std::string &entryPoint, std::vector<uint8_t> &binary, std::string &error)
{
	std::wstring lEntryPoint = {entryPoint.begin(), entryPoint.end()};
    std::vector<LPCWSTR> arguments = {
        L"name",
	    L"-E", lEntryPoint.c_str(),
        L"-T", GetShaderTarget(stage)
    };

    if (binaryType == ShaderBinaryType::SPIRV)
    {
		arguments.emplace_back(L"-spirv");
    }

    DxcBuffer buffer = {
	    buffer.Ptr      = data,
	    buffer.Size     = size,
	    buffer.Encoding = DXC_CP_ACP
    };

    ComPtr<IDxcResult> result{nullptr};
    auto hres = compiler->Compile(
        &buffer,
        arguments.data(),
        (uint32_t) arguments.size(),
        nullptr,
        IID_PPV_ARGS(&result)
    );

    if (SUCCEEDED(hres))
    {
        result->GetStatus(&hres);
    }

    if (FAILED(hres) && (result))
    {
        ComPtr<IDxcBlobEncoding> errorBlob;
        hres = result->GetErrorBuffer(&errorBlob);
        if (SUCCEEDED(hres) && errorBlob)
        {
			error = std::move(std::string((const char *) errorBlob->GetBufferPointer()));
			return false;
        }
    }

    ComPtr<IDxcBlob> code;
    result->GetResult(&code);

    binary.resize(code->GetBufferSize());
	memcpy(binary.data(), code->GetBufferPointer(), binary.size());

    return true;
}

bool DirectXShaderCompiler::Reflect(ShaderBinaryType binaryType, const std::vector<uint8_t> &binary, REFIID iid, void **ppvReflection)
{
    DxcBuffer buffer = {
         .Ptr     = binary.data(),
	    .Size     = binary.size(),
        .Encoding = 0,
    };

    if (FAILED(utils->CreateReflection(&buffer, iid, ppvReflection)))
    {
		LOG::ERR("Failed to create shader reflection!");
		return false;
    }

    return true;
}

};
