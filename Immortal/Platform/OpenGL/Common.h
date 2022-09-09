#pragma once

#ifndef __OPENGL_COMMON_H__
#define __OPENGL_COMMON_H__

#include "Core.h"
#include <glad/glad.h>

#define GLCPP_OPERATOR_HANDLE() using Primitive = uint32_t; Primitive Handle() const { return handle; } operator Primitive() const { return handle; } protected: Primitive handle = 0;

#define STR(x) #x
#define MAEK_STR(x) STR(x)
#define PUSH_CONSTANT_LOCATION 33
#define PUSH_CONSTANT_LOCATION_STR MAEK_STR(PUSH_CONSTANT_LOCATION)

#endif /* __OPENGL_COMMON_H__ */
