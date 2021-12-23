#pragma once

#include "Common.h"
#include "Render/RenderTarget.h"
#include "Render/Pipeline.h"
#include "Render/Texture.h"
#include "Shader.h"
#include "RootSignature.h"

namespace Immortal
{
namespace D3D12
{

class Device;
class Pipeline : public SuperPipeline
{
public:
    using Super = SuperPipeline;

    struct State
    {
        std::vector<D3D12_INPUT_ELEMENT_DESC> InputElementDescription;
    };

    struct RasterizerDescription : public D3D12_RASTERIZER_DESC
    {
        using Primitive = D3D12_RASTERIZER_DESC;

        RasterizerDescription()
        {
            Primitive::FillMode              = D3D12_FILL_MODE_SOLID;
            Primitive::CullMode              = D3D12_CULL_MODE_BACK;
            Primitive::FrontCounterClockwise = FALSE;
            Primitive::DepthBias             = D3D12_DEFAULT_DEPTH_BIAS;
            Primitive::DepthBiasClamp        = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
            Primitive::SlopeScaledDepthBias  = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
            Primitive::DepthClipEnable       = TRUE;
            Primitive::MultisampleEnable     = FALSE;
            Primitive::AntialiasedLineEnable = FALSE;
            Primitive::ForcedSampleCount     = 0;
            Primitive::ConservativeRaster    = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
        }

        explicit RasterizerDescription(const Primitive &o) noexcept :
            Primitive{ o }
        {
        
        }

        explicit RasterizerDescription(
            D3D12_FILL_MODE fillMode,
            D3D12_CULL_MODE cullMode,
            BOOL frontCounterClockwise,
            INT depthBias,
            FLOAT depthBiasClamp,
            FLOAT slopeScaledDepthBias,
            BOOL depthClipEnable,
            BOOL multisampleEnable,
            BOOL antialiasedLineEnable,
            UINT forcedSampleCount,
            D3D12_CONSERVATIVE_RASTERIZATION_MODE conservativeRaster) noexcept
        {
            Primitive::FillMode              = fillMode;
            Primitive::CullMode              = cullMode;
            Primitive::FrontCounterClockwise = frontCounterClockwise;
            Primitive::DepthBias             = depthBias;
            Primitive::DepthBiasClamp        = depthBiasClamp;
            Primitive::SlopeScaledDepthBias  = slopeScaledDepthBias;
            Primitive::DepthClipEnable       = depthClipEnable;
            Primitive::MultisampleEnable     = multisampleEnable;
            Primitive::AntialiasedLineEnable = antialiasedLineEnable;
            Primitive::ForcedSampleCount     = forcedSampleCount;
            Primitive::ConservativeRaster    = conservativeRaster;
        }
    };

    struct BlendDescription : public D3D12_BLEND_DESC
    {
        using Primitive = D3D12_BLEND_DESC;

        explicit BlendDescription() noexcept
        {
            Primitive::AlphaToCoverageEnable  = FALSE;
            Primitive::IndependentBlendEnable = FALSE;
            const D3D12_RENDER_TARGET_BLEND_DESC defaultRenderTargetBlendDescription =
            {
                FALSE,FALSE,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
                D3D12_LOGIC_OP_NOOP,
                D3D12_COLOR_WRITE_ENABLE_ALL,
            };
            for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
            {
                Primitive::RenderTarget[i] = defaultRenderTargetBlendDescription;
            }
        }

        explicit BlendDescription(const D3D12_BLEND_DESC &o) noexcept :
            Primitive{ o }
        {
        
        }
    };

    static D3D12_PRIMITIVE_TOPOLOGY_TYPE SuperToBase(const PrimitiveType type)
    {
        switch (type)
        {
        case PrimitiveType::Line:
            return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;

        case PrimitiveType::Point:
            return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;

        case PrimitiveType::Triangles:
        default:
            return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        }
    }

public:
    Pipeline(Device *device, std::shared_ptr<Shader::Super> shader);

    virtual ~Pipeline();

    virtual void Create(const std::shared_ptr<SuperRenderTarget> &renderTarget) override;

    virtual void Reconstruct(const std::shared_ptr<SuperRenderTarget> &renderTarget) override;

    virtual void Set(const InputElementDescription &description) override;

private:
    Device *device{ nullptr };

    ID3D12PipelineState *pipelineState{ nullptr };

    RootSignature rootSignature;

    std::unique_ptr<State> state;
};

}
}
