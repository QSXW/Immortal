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

    void Set(const std::string &name, int value);

    void Set(const std::string &name, int *values, uint32_t count);

    void Set(const std::string &name, float value);

    void Set(const std::string &name, const Vector2 &value);

    void Set(const std::string &name, const Vector3 &value);

    void Set(const std::string &name, const Vector4 &value);

    void Set(const std::string &name, const Matrix4 &value);

    virtual void DispatchCompute(uint32_t numGroupsX, uint32_t numGroupsY, uint32_t numGroupsZ);

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
