#pragma once

#include "Common.h"
#include "Handle.h"
#include "Graphics/Shader.h"
#include "Algorithm/LightArray.h"

namespace Immortal
{
namespace D3D11
{

class Device;
class IMMORTAL_API Shader : public SuperShader, public NonDispatchableHandle
{
public:
    using Super = SuperShader;
	using Primitive = ID3D11DeviceChild;
	D3D11_OPERATOR_HANDLE()
	D3D11_SWAPPABLE(Shader)

public:
	Shader();

	Shader(Device *device, const std::string &name, ShaderSourceType sourceType, ShaderStage stage, const std::string &source, const std::string &entryPoint);

    virtual ~Shader() override;

	void LoadByteCodesFromSource(const std::string &name, ShaderStage stage, const std::string &source, const std::string &entryPoint);

public:
	D3D_PUSH_CONSTANT_DESC GetPushConstantDesc() const
	{
		return pushConstant;
	}

	ID3DBlob *GetByteCodes() const
	{
		return byteCodes.Get();
	}

	const Stage &GetStage() const
	{
		return stage;
	}

	template <class T>
	ComPtr<T> Get()
	{
		ComPtr<T> ret{};
		Check(handle.As<T>(&ret));
		return ret;
	}

	void Swap(Shader &other)
	{
		NonDispatchableHandle::Swap(other);
		std::swap(handle,       other.handle      );
		std::swap(byteCodes,    other.byteCodes   );
		std::swap(stage,        other.stage       );
		std::swap(pushConstant, other.pushConstant);
	}

protected:
    void Reflect(ID3D11ShaderReflection *shaderRelection);

	void ConstructHandle(const void *bufferPointer, size_t bufferSize);

protected:
	ComPtr<ID3DBlob> byteCodes;

	ShaderStage stage;

	D3D_PUSH_CONSTANT_DESC pushConstant;
};

}
}
