#pragma once

#ifndef __OPENGL_COMMON_H__
#define __OPENGL_COMMON_H__

#include "Core.h"
#include "GLFormat.h"

#include <glad/glad.h>

#define GLCPP_OPERATOR_HANDLE() using Primitive = uint32_t; Primitive Handle() const { return handle; } operator Primitive() const { return handle; } protected: Primitive handle = 0;

#define PUSH_CONSTANT_LOCATION 33
#define PUSH_CONSTANT_LOCATION_STR SL_MAEK_STR(PUSH_CONSTANT_LOCATION)

namespace Immortal
{
namespace OpenGL
{

enum GLFilter : GLenum
{
    GL_FILTER_LINEAR  = GL_LINEAR,
    GL_FILTER_NEAREST = GL_NEAREST,
};

enum GLSampler : GLenum
{
    GL_SAMPLER_REPEAT          = GL_REPEAT,
    GL_SAMPLER_CLAMP_TO_BORDER = GL_CLAMP_TO_BORDER,
    GL_SAMPLER_CLAMP_TO_EDGE   = GL_CLAMP_TO_EDGE,
};

#define GLCPP_SWAPPABLE(T) T(T &&other) : T{} { other.Swap(*this); } T &operator=(T &&other) { T(std::move(other)).Swap(*this); return *this; }  T(const T &) = delete; T &operator=(const T &) = delete;

class Handle
{
public:
	static constexpr uint32_t Invalid = 0;
 
public:
    Handle(uint32_t handle = 0) :
        handle{ handle }
    {

    }

    operator uint32_t() const
    {
        return handle;
    }

    void Swap(Handle &other)
    {
		std::swap(handle, other.handle);
    }

protected:
    uint32_t handle;
};

}
}

#endif /* __OPENGL_COMMON_H__ */
