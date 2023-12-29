#pragma once

#include "Core.h"
#include "Render/Shader.h"
#include "Common.h"
#include "Math/Vector.h"

namespace Immortal
{
namespace OpenGL
{

class Shader : public SuperShader, public Handle
{
public:
    using Super = SuperShader;

public:
	Shader(const std::string &name, ShaderStage stage, const std::string &source, const std::string &entryPoint);

    virtual ~Shader() override;

    void Activate() const;

    void Deactivate() const;

    GLuint Get(const std::string &name) const;

protected:
    void Link(const std::vector<GLuint> &shaders);

    uint32_t CompileShader(GLenum type, const std::string &source);

    std::string InjectPreamble(const std::string &source);

protected:
    std::string name;

    GLenum stage;
};

}
}
