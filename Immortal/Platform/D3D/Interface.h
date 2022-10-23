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
    }                                  \
                                       \
    Primitive **AddressOf()            \
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

static inline const char *TypeString(DXGI_FORMAT format)
{
#define CASE(x) case x: return #x;
    switch (format)
    {
        CASE(DXGI_FORMAT_UNKNOWN                                )        
        CASE(DXGI_FORMAT_R32G32B32A32_TYPELESS					)
        CASE(DXGI_FORMAT_R32G32B32A32_FLOAT						)
        CASE(DXGI_FORMAT_R32G32B32A32_UINT						)
        CASE(DXGI_FORMAT_R32G32B32A32_SINT						)
        CASE(DXGI_FORMAT_R32G32B32_TYPELESS						)
        CASE(DXGI_FORMAT_R32G32B32_FLOAT						)
        CASE(DXGI_FORMAT_R32G32B32_UINT							)
        CASE(DXGI_FORMAT_R32G32B32_SINT							)
        CASE(DXGI_FORMAT_R16G16B16A16_TYPELESS					)
        CASE(DXGI_FORMAT_R16G16B16A16_FLOAT 					)
        CASE(DXGI_FORMAT_R16G16B16A16_UNORM 					)
        CASE(DXGI_FORMAT_R16G16B16A16_UINT 						)
        CASE(DXGI_FORMAT_R16G16B16A16_SNORM 					)
        CASE(DXGI_FORMAT_R16G16B16A16_SINT 						)
        CASE(DXGI_FORMAT_R32G32_TYPELESS 						)
        CASE(DXGI_FORMAT_R32G32_FLOAT 							)
        CASE(DXGI_FORMAT_R32G32_UINT 							)
        CASE(DXGI_FORMAT_R32G32_SINT 							)
        CASE(DXGI_FORMAT_R32G8X24_TYPELESS 						)
        CASE(DXGI_FORMAT_D32_FLOAT_S8X24_UINT 					)
        CASE(DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS 				)
        CASE(DXGI_FORMAT_X32_TYPELESS_G8X24_UINT 				)
        CASE(DXGI_FORMAT_R10G10B10A2_TYPELESS 					)
        CASE(DXGI_FORMAT_R10G10B10A2_UNORM 						)
        CASE(DXGI_FORMAT_R10G10B10A2_UINT 						)
        CASE(DXGI_FORMAT_R11G11B10_FLOAT 						)
        CASE(DXGI_FORMAT_R8G8B8A8_TYPELESS 						)
        CASE(DXGI_FORMAT_R8G8B8A8_UNORM 						)
        CASE(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB 					)
        CASE(DXGI_FORMAT_R8G8B8A8_UINT 							)
        CASE(DXGI_FORMAT_R8G8B8A8_SNORM 						)
        CASE(DXGI_FORMAT_R8G8B8A8_SINT 							)
        CASE(DXGI_FORMAT_R16G16_TYPELESS 						)
        CASE(DXGI_FORMAT_R16G16_FLOAT 							)
        CASE(DXGI_FORMAT_R16G16_UNORM 							)
        CASE(DXGI_FORMAT_R16G16_UINT 							)
        CASE(DXGI_FORMAT_R16G16_SNORM 							)
        CASE(DXGI_FORMAT_R16G16_SINT 							)
        CASE(DXGI_FORMAT_R32_TYPELESS 							)
        CASE(DXGI_FORMAT_D32_FLOAT 								)
        CASE(DXGI_FORMAT_R32_FLOAT 								)
        CASE(DXGI_FORMAT_R32_UINT 								)
        CASE(DXGI_FORMAT_R32_SINT 								)
        CASE(DXGI_FORMAT_R24G8_TYPELESS 						)
        CASE(DXGI_FORMAT_D24_UNORM_S8_UINT 						)
        CASE(DXGI_FORMAT_R24_UNORM_X8_TYPELESS 					)
        CASE(DXGI_FORMAT_X24_TYPELESS_G8_UINT 					)
        CASE(DXGI_FORMAT_R8G8_TYPELESS 							)
        CASE(DXGI_FORMAT_R8G8_UNORM 							)
        CASE(DXGI_FORMAT_R8G8_UINT 								)
        CASE(DXGI_FORMAT_R8G8_SNORM 							)
        CASE(DXGI_FORMAT_R8G8_SINT 								)
        CASE(DXGI_FORMAT_R16_TYPELESS 							)
        CASE(DXGI_FORMAT_R16_FLOAT 								)
        CASE(DXGI_FORMAT_D16_UNORM 								)
        CASE(DXGI_FORMAT_R16_UNORM 								)
        CASE(DXGI_FORMAT_R16_UINT 								)
        CASE(DXGI_FORMAT_R16_SNORM 								)
        CASE(DXGI_FORMAT_R16_SINT 								)
        CASE(DXGI_FORMAT_R8_TYPELESS 							)
        CASE(DXGI_FORMAT_R8_UNORM 								)
        CASE(DXGI_FORMAT_R8_UINT 								)
        CASE(DXGI_FORMAT_R8_SNORM 								)
        CASE(DXGI_FORMAT_R8_SINT 								)
        CASE(DXGI_FORMAT_A8_UNORM 								)
        CASE(DXGI_FORMAT_R1_UNORM 								)
        CASE(DXGI_FORMAT_R9G9B9E5_SHAREDEXP 					)
        CASE(DXGI_FORMAT_R8G8_B8G8_UNORM 						)
        CASE(DXGI_FORMAT_G8R8_G8B8_UNORM 						)
        CASE(DXGI_FORMAT_BC1_TYPELESS 							)
        CASE(DXGI_FORMAT_BC1_UNORM 								)
        CASE(DXGI_FORMAT_BC1_UNORM_SRGB 						)
        CASE(DXGI_FORMAT_BC2_TYPELESS 							)
        CASE(DXGI_FORMAT_BC2_UNORM 								)
        CASE(DXGI_FORMAT_BC2_UNORM_SRGB 						)
        CASE(DXGI_FORMAT_BC3_TYPELESS 							)
        CASE(DXGI_FORMAT_BC3_UNORM 								)
        CASE(DXGI_FORMAT_BC3_UNORM_SRGB 						)
        CASE(DXGI_FORMAT_BC4_TYPELESS 							)
        CASE(DXGI_FORMAT_BC4_UNORM 								)
        CASE(DXGI_FORMAT_BC4_SNORM 								)
        CASE(DXGI_FORMAT_BC5_TYPELESS 							)
        CASE(DXGI_FORMAT_BC5_UNORM 								)
        CASE(DXGI_FORMAT_BC5_SNORM 								)
        CASE(DXGI_FORMAT_B5G6R5_UNORM 							)
        CASE(DXGI_FORMAT_B5G5R5A1_UNORM 						)
        CASE(DXGI_FORMAT_B8G8R8A8_UNORM 						)
        CASE(DXGI_FORMAT_B8G8R8X8_UNORM 						)
        CASE(DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM 			)
        CASE(DXGI_FORMAT_B8G8R8A8_TYPELESS 						)
        CASE(DXGI_FORMAT_B8G8R8A8_UNORM_SRGB 					)
        CASE(DXGI_FORMAT_B8G8R8X8_TYPELESS 						)
        CASE(DXGI_FORMAT_B8G8R8X8_UNORM_SRGB 					)
        CASE(DXGI_FORMAT_BC6H_TYPELESS 							)
        CASE(DXGI_FORMAT_BC6H_UF16 								)
        CASE(DXGI_FORMAT_BC6H_SF16 								)
        CASE(DXGI_FORMAT_BC7_TYPELESS 							)
        CASE(DXGI_FORMAT_BC7_UNORM 								)
        CASE(DXGI_FORMAT_BC7_UNORM_SRGB 						)
        CASE(DXGI_FORMAT_AYUV 									)
        CASE(DXGI_FORMAT_Y410 									)
        CASE(DXGI_FORMAT_Y416 									)
        CASE(DXGI_FORMAT_NV12 									)
        CASE(DXGI_FORMAT_P010 									)
        CASE(DXGI_FORMAT_P016 									)
        CASE(DXGI_FORMAT_420_OPAQUE 							)
        CASE(DXGI_FORMAT_YUY2 									)
        CASE(DXGI_FORMAT_Y210 									)
        CASE(DXGI_FORMAT_Y216 								    )
        CASE(DXGI_FORMAT_NV11 									)
        CASE(DXGI_FORMAT_AI44 									)
        CASE(DXGI_FORMAT_IA44    								)
        CASE(DXGI_FORMAT_P8 									)
        CASE(DXGI_FORMAT_A8P8 									)
        CASE(DXGI_FORMAT_B4G4R4A4_UNORM 						)

        CASE(DXGI_FORMAT_P208    								)
        CASE(DXGI_FORMAT_V208 									)
        CASE(DXGI_FORMAT_V408   								)

        CASE(DXGI_FORMAT_SAMPLER_FEEDBACK_MIN_MIP_OPAQUE  		)
        CASE(DXGI_FORMAT_SAMPLER_FEEDBACK_MIP_REGION_USED_OPAQUE)

        CASE(DXGI_FORMAT_FORCE_UINT             		     	)
		default:
			return "DXGI_FORMAT_UNKNOWN";
    }
#undef CASE
}

static inline void Guid2String(char *dst, const GUID &guid)
{
	sprintf(dst, "%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
	        guid.Data1,
	        guid.Data2,
	        guid.Data3,
	        guid.Data4[0], guid.Data4[1],
	        guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
}

}
}
