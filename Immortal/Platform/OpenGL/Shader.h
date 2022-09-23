#pragma once

#include "Core.h"
#include "Render/Shader.h"
#include "Common.h"
#include "Math/Vector.h"

namespace Immortal
{
namespace OpenGL
{

class Shader : public SuperShader
{
public:
    using Super = SuperShader;
	GLCPP_OPERATOR_HANDLE()

public:
    Shader(const std::string &filepath, Type type);

    ~Shader();

    void Activate() const;

    void Deactivate() const;

    virtual const char *Name() const override
    {
        return name.c_str();
    }

    GLuint Get(const std::string &name) const;

private:
    void Link(const std::vector<GLuint> &shaders);

    uint32_t CompileShader(GLenum type, const std::string &source);

    std::string __InjectPreamble(const std::string &source);

private:
    std::string name;

    Shader::Type type = Shader::Type::Graphics;
};

}
}
