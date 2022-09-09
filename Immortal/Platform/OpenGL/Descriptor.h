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
	Image,
	ImageWrite,
};

struct Descriptor
{
	Descriptor(uint32_t handle, DescriptorType type, uint32_t binding) :
	    Type{ type },
	    Binding{ binding },
	    handle{handle}
	{

	}

	DescriptorType Type;

	uint32_t Binding;

	GLCPP_OPERATOR_HANDLE()
};

}
}
