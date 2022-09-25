#pragma once

#include <wrl/client.h>
using Microsoft::WRL::ComPtr;

#include <initguid.h>
#include <dxgi.h>
#include <d3dcommon.h>
#include <d3dcompiler.h>
#pragma comment(lib, "dxgi.lib")

namespace Immortal
{
namespace D3D
{

#define D3D_OPERATOR_PRIMITIVE(handle) \
protected:                             \
    ComPtr<Primitive> handle;          \
                                       \
public:                                \
    Primitive *Handle() const          \
    {                                  \
        return handle.Get();           \
    }                                  \
                                       \
    operator Primitive*() const        \
    {                                  \
        return handle.Get();           \
    }                                  \
                                       \
    Primitive **operator &()           \
    {                                  \
        return handle.GetAddressOf();  \
    }

#define D3D_OPERATOR_HANDLE() D3D_OPERATOR_PRIMITIVE(handle)


static inline void Check(HRESULT result, const char *message = "")
{
    if (FAILED(result))
    {
        LOG::ERR("Status Code => {}", GetLastError());
        if (!message || !message[0])
        {
            LOG::ERR("{}", "This is a DirectX 3D Execption. Check Output for more details...");
        }
        else
        {
            LOG::ERR("{}", message);
        }

        throw RuntimeException(message);
    }
}

struct ShaderByteCodes
{
public:
    ShaderByteCodes() :
	    Graphics{}
    {

    }

    ~ShaderByteCodes()
    {

    }

    union
    {
		ComPtr<ID3DBlob> Compute;

        struct
        {
			ComPtr<ID3DBlob> Vertex;
			ComPtr<ID3DBlob> Pixel;
        } Graphics;
    };
};

struct D3D_PUSH_CONSTANT_DESC
{
	D3D_PUSH_CONSTANT_DESC() :
	    Size{},
        Binding{}
    {

    }

    operator bool() const
    {
		return Size > 0;
    }

	uint32_t Size;
	UINT Binding;
};

}
}
