#pragma once

#include "Common.h"
#include "Render/Texture.h"

namespace Immortal
{
namespace D3D11
{

class Device;
class Sampler
{
public:
    using Primitive = ID3D11SamplerState;
    D3D11_OPERATOR_HANDLE()

public:
	Sampler(Device *device, const Texture::Description &desc);
};

}
}
