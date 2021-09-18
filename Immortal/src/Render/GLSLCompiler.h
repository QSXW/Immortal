#pragma once

#include "GLSLCompiler.h"
#include "Shader.h"

namespace Immortal
{

class GLSLCompiler
{
public:
    static bool Src2Spirv(Shader::API                 api,
                          Shader::Stage               stage,
                          const std::vector<uint8_t> &src,
                          const char                 *entryPoint,
                          std::vector<uint32_t>      &spriv,
                          std::string                &error);
};

};
