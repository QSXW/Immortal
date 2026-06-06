#pragma once

#include "Core.h"

namespace Immortal
{

static constexpr uint32_t TextureAlignment = 256;

enum class BackendAPI
{
    None,
    D3D11,
    D3D12,
    Vulkan,
    Metal,
	OpenGL
};

enum class SwapchainMode
{
    None         = 0,
    VerticalSync = 1,
};

enum class PhysicalDeviceType
{
    Software,
    Discrete,
    Integrate,
};
SL_ENABLE_BITWISE_OPERATOR(PhysicalDeviceType)

enum class QueueType
{
    None           = BIT(31),
    Graphics       = BIT(0),
    Compute        = BIT(1),
    Transfer       = BIT(2),
    SparseBiding   = BIT(3),
    VideoDecoding  = BIT(5),
    VideoEncoding  = BIT(6)
};
SL_ENABLE_BITWISE_OPERATOR(QueueType)

enum class QueuePriority
{
    Normal         = 0,
    High           = 100,
    GlobalRealTime = 10000,
};
SL_ENABLE_BITWISE_OPERATOR(QueuePriority)

enum class CommandBufferType
{
    None,
    Primary,
    Secondary,
};
SL_ENABLE_BITWISE_OPERATOR(CommandBufferType)

enum class BufferType
{
    None                          = 0,
    Vertex                        = BIT(0),
    Index                         = BIT(1),
    ConstantBuffer                = BIT(2),
    PushConstant                  = BIT(3),
    TransferSource                = BIT(4),
    TransferDestination           = BIT(5),
    Storage                       = BIT(6),
    VideoDecoder                  = BIT(7),
    VideoEncoder                  = BIT(8),
    Scratch                       = BIT(9),
    AccelerationStructure         = BIT(10),
    AccelerationStructureSource   = BIT(11),
    ASVertex                      = AccelerationStructureSource | Vertex,
    ASIndex                       = AccelerationStructureSource | Index,
    Instance                      = BIT(11),
};
SL_ENABLE_BITWISE_OPERATOR(BufferType)

enum class TextureType
{
    None                   = 0,
    TransferSource         = BIT(0),
    TransferDestination    = BIT(1),
    Sampled                = BIT(2),
    Storage                = BIT(3),
    ColorAttachment        = BIT(4),
    DepthStencilAttachment = BIT(5),
};
SL_ENABLE_BITWISE_OPERATOR(TextureType)

enum class PipelineType
{
    None     = BIT(31),
    Graphics = BIT(0),
    Compute  = BIT(1),
    Meshlet  = BIT(2)
};
SL_ENABLE_BITWISE_OPERATOR(PipelineType)

enum class ShaderStage
{
    Vertex                = BIT(0),
    TesselationControl    = BIT(1),
    TesselationEvaluation = BIT(2),
    Geometry              = BIT(3),
    Fragment              = BIT(4),
    Compute               = BIT(5),
    RayGen                = BIT(6),
    AnyHit                = BIT(7),
    ClosestHit            = BIT(8),
    Miss                  = BIT(9),
    Intersection          = BIT(10),
    Callable              = BIT(11),
    Mesh                  = BIT(12),
    Pixel                 = Fragment,
    Unspecified           = BIT(31)
};
SL_ENABLE_BITWISE_OPERATOR(ShaderStage)

enum class ShaderSourceType
{
    GLSL,
    HLSL,
    MSL
};

enum class ShaderBinaryType
{
    SPIRV,
    DXIL,
};

enum class ShaderCompilerType
{
    DirectXShaderCompiler,
    glslang
};

enum class AddressMode
{
    None        = 0,
    Wrap        = 1,
    Mirror      = 2,
    Clamp       = 3,
    BorderColor = 4,
    Repeat      = Wrap,
};
using Wrap = AddressMode;

enum class Filter
{
    None = 0,
    Linear,
    Bilinear,
    Nearest,
    Anisotropic
};

enum class CompareOperation
{
    Never          = 0,
    Less           = 1,
    Equal          = 2,
    LessOrEqual    = 3,
    Greater        = 4,
    NotEqual       = 5,
    GreaterOrEqual = 6,
    Always         = 7,
};

struct Rect2D
{
    uint32_t left;
    uint32_t top;
    uint32_t right;
    uint32_t bottom;
};

struct Size3D
{
    uint32_t x;
    uint32_t y;
    uint32_t z;
};

struct DeviceAddressRegion
{
	uint64_t address;
	uint64_t stride;
	uint64_t size;
};

struct BufferBindInfo
{
	BufferType type;
	uint32_t size;
	uint32_t offset;
};

}
