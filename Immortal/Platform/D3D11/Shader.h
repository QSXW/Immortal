#pragma once

#include "Common.h"
#include "Device.h"
#include "Render/Shader.h"
#include "Algorithm/LightArray.h"

namespace Immortal
{
namespace D3D11
{

template <class T>
concept ID3D11Shader =
    std::is_same_v<T, ID3D11VertexShader> ||
    std::is_same_v<T, ID3D11PixelShader>  ||
    std::is_same_v<T, ID3D11ComputeShader>;

class Shader : public SuperShader
{
public:
    using Super = SuperShader;

public:
    Shader(Device *device, const std::string &filepath, Type type = Type::Graphics);

    Shader(Device *device, const std::string &name, const std::string &source, Type type = Type::Graphics);

    virtual ~Shader();

    void LoadByteCodes(Device *device, const std::string &source, const std::string &name, Type type = Type::Graphics);

    template <ID3D11Shader T>
    void QueryInterface(ComPtr<T> *pShader)
    {
		if constexpr (std::is_same_v<T, ID3D11VertexShader> || std::is_same_v<T, ID3D11ComputeShader>)
		{
			handles[0].As<T>(pShader);
		}
		else if constexpr (std::is_same_v<T, ID3D11PixelShader>)
		{
			handles[1].As<T>(pShader);
		}
    }

	void GetDesc(D3D_PUSH_CONSTANT_DESC *pDesc) const
	{
		*pDesc = pushConstants;
	}

	ID3DBlob *GetVSByteCoeds() const
	{
		return VSByteCodes.Get();
	}

protected:
    void __Check(HRESULT result, ID3DBlob **errorMsg, ID3DBlob **toBeReleased);

    void __Reflect(ID3DBlob *byteCodes);

    template <ID3D11Shader T, size_t index>
	void __Create(Device *device, ID3DBlob *bytesCodes)
	{
		ComPtr<T> pShader;
        if constexpr (std::is_same_v<T, ID3D11VertexShader>)
        {
			Check(device->CreateVertexShader(bytesCodes->GetBufferPointer(), bytesCodes->GetBufferSize(), nullptr, &pShader));
        }
		else if constexpr (std::is_same_v<T, ID3D11PixelShader>)
		{
			Check(device->CreatePixelShader(bytesCodes->GetBufferPointer(), bytesCodes->GetBufferSize(), nullptr, &pShader));
		}
		else if constexpr (std::is_same_v<T, ID3D11ComputeShader>)
		{
			Check(device->CreateComputeShader(bytesCodes->GetBufferPointer(), bytesCodes->GetBufferSize(), nullptr, &pShader));
		}

        pShader.As<ID3D11DeviceChild>(&handles[index]);
    }

private:
	ComPtr<ID3DBlob> VSByteCodes;

	std::array<ComPtr<ID3D11DeviceChild>, 2> handles;

	D3D_PUSH_CONSTANT_DESC pushConstants;
};

}
}
