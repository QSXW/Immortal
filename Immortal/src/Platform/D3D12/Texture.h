#pragma once

#include "D3D12Common.h"
#include "Descriptor.h"
#include "Device.h"

#include "Render/Texture.h"
#include "Render/RenderContext.h"

namespace Immortal
{
namespace D3D12
{

class Texture : public SuperTexture2D
{
public:
    using Super = SuperTexture2D;

public:
    Texture(SuperRenderContext *context, const std::string &filepath, bool flip = false);

private:
    D3D12_GPU_DESCRIPTOR_HANDLE handle{};
    
    D3D12_RESOURCE_DESC desc{};
};

}
}
