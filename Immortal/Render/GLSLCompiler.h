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
                          uint32_t                    size,
                          const char                 *data,
                          const char                 *entryPoint,
                          std::vector<uint32_t>      &spriv,
                          std::string                &error);
};

};
