#include "Shader.h"

#include "Common.h"
#include <GLFW/glfw3.h>

#include "FileSystem/FileSystem.h"
#include "Graphics/DirectXShaderCompiler.h"
#include "Shared/Log.h"

namespace Immortal
{
namespace OpenGL
{

static constexpr size_t MAX_SHADER_SOURCE = 2;

GLenum CAST(ShaderStage stage)
{
	switch (stage)
	{
		case ShaderStage::Vertex:
			return GL_VERTEX_SHADER;
		case ShaderStage::TesselationControl:
			return GL_TESS_CONTROL_SHADER;
		case ShaderStage::TesselationEvaluation:
			return GL_TESS_EVALUATION_SHADER;
		case ShaderStage::Geometry:
			return GL_GEOMETRY_SHADER;
		case ShaderStage::Fragment:
			return GL_FRAGMENT_SHADER;
		case ShaderStage::Compute:
			return GL_COMPUTE_SHADER;
		default:
			return GL_INVALID_ENUM;
	}
}

Shader::Shader(const std::string &name, ShaderStage _stage, const std::string &source, const std::string &entryPoint) :
    Handle{},
    stage{ CAST(_stage) }
{
	std::string error;
	std::vector<uint8_t> spriv;
	DirectXShaderCompiler compiler;
	if (compiler.Compile(name, ShaderSourceType::HLSL, ShaderBinaryType::SPIRV, _stage, source.size(), source.data(), entryPoint, spriv, error))
    {
		handle = glCreateShader(stage);
		glShaderBinary(1, &handle, GL_SHADER_BINARY_FORMAT_SPIR_V, spriv.data(), spriv.size());
		glSpecializeShader(handle, entryPoint.c_str(), 0, 0, 0);
    }
    else
    {
		handle = CompileShader(stage, source);
    }

	int status = 0;
	glGetShaderiv(handle, GL_COMPILE_STATUS, &status);
	if (!status)
    {
		throw std::runtime_error("Failed to init shader!");
    }
}

Shader::~Shader()
{
    if (handle)
    {
		glDeleteShader(handle);
		handle = 0;
    }
}

GLuint Shader::Get(const std::string &name) const
{
    Activate();
    GLuint location = glGetUniformLocation(handle, name.c_str());
    Deactivate();

    return location;
}

void Shader::Link(const std::vector<GLuint> &shaders)
{
    handle = glCreateProgram();

    for (auto &shader : shaders)
    {
        glAttachShader(handle, shader);
    }
    glLinkProgram(handle);

    GLint status = GL_TRUE;
    glGetProgramiv(handle, GL_LINK_STATUS, &status);
    if (status == GL_FALSE)
    {
        GLint maxLength = 0;
		glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &maxLength);

        std::vector<GLchar> error;
        error.resize(maxLength);
        glGetProgramInfoLog(handle, maxLength, &maxLength, error.data());

        LOG::ERR("ShaderProgram::{} link failure!", name);
        LOG::ERR("{}", error.data());
    }

    for (auto &shader : shaders)
    {
        glDetachShader(handle, shader);
        glDeleteShader(shader);
    }
}

std::string Shader::InjectPreamble(const std::string &source)
{
	static std::string preamble = "#define push_constant binding=" PUSH_CONSTANT_LOCATION_STR "\n#define " PLATFORM_STRING "\n";
	size_t index = source.find("layout");
	return source.substr(0, index) + preamble + source.substr(index);
}

uint32_t Shader::CompileShader(GLenum type, const std::string &source)
{
	std::string shaderSource = InjectPreamble(source);
    const char *pSource = shaderSource.c_str();

    uint32_t shader = glCreateShader(type);
	glShaderSource(shader, 1, &pSource, 0);
    glCompileShader(shader);

    GLint isCompiled = GL_TRUE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
    if (isCompiled == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

        std::vector<GLchar> error;
        error.resize(maxLength);
        glGetShaderInfoLog(shader, maxLength, &maxLength, error.data());
        glDeleteShader(shader);

        LOG::ERR("Shader compilation failure!");
        LOG::ERR("{}", error.data());

        THROWIF(true, error.data());
    }

    return shader;
}

void Shader::Activate() const
{
    glUseProgram(handle);
}

void Shader::Deactivate() const
{
    glUseProgram(0);
}

}
}
