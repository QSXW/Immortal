#pragma once

#include "ImmortalCore.h"
#include "Render/Shader.h"
#include "Common.h"

namespace Immortal
{
namespace OpenGL
{

class Shader : public SuperShader
{
public:
    using Super = SuperShader;

public:
    Shader(const std::string &filepath, Type type);

    Shader(const std::string &name, const std::string &vertexSrc, const std::string &fragmentSrc);

    ~Shader();

    void Map() const override;

    void Unmap() const override;

    void Set(const std::string &name, int value) override;

    void Set(const std::string &name, int *values, uint32_t count) override;

    void Set(const std::string &name, float value) override;

    void Set(const std::string &name, const Vector2 &value) override;

    void Set(const std::string &name, const Vector3 &value) override;

    void Set(const std::string &name, const Vector4 &value) override;

    void Set(const std::string &name, const Matrix4 &value) override;

    virtual void DispatchCompute(uint32_t numGroupsX, uint32_t numGroupsY, uint32_t numGroupsZ);

    virtual const uint32_t Handle() const override
    {
        return handle;
    }

    virtual const char *Name() const override
    {
        return name.c_str();
    }

    operator GLint() const
    {
        return handle;
    }

    GLuint Get(const std::string &name) const;

private:
    void Compile(const std::unordered_map<GLenum, std::string> &source);

    uint32_t CompileShader(int type, const char *src);

private:
    uint32_t handle;

    std::string name;

    Shader::Type type = Shader::Type::Graphics;
};

}
}
