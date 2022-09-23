#pragma once

#include "Common.h"
#include "Render/Descriptor.h"

namespace Immortal
{
namespace OpenGL
{

enum class DescriptorType
{
	Buffer,
	Sampler = GL_SAMPLER_2D,
	Image2D = GL_IMAGE_2D,
};

struct Descriptor
{
	uint32_t handle;

	DescriptorType Type;

	uint32_t Binding;
};

}
}
