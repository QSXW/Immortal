#pragma once

#include "Common.h"

namespace Immortal
{
namespace OpenGL
{

enum class DescriptorType
{
    None,
    UniformBuffer,
    Sampler,
    SamplerImage2D,
    Image2D,
    MaxTypes
};

struct Descriptor
{
    uint32_t handle;

    uint32_t binding;

    DescriptorType type;

    GL_FORMAT format;
};

}
}
