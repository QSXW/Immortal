#pragma once

#include "GLSLCompiler.h"
#include "Shader.h"

namespace Immortal
{

class GLSLCompiler
{
public:
    static bool Src2Spirv(Shader::API api, Shader::Stage stage, const std::vector<UINT8> &src, const char *entryPoint, const std::vector<UINT8> spriv, std::string &error);
};

};
