#include "Shader.h"

#include "Common.h"
#include <GLFW/glfw3.h>

#include "FileSystem/FileSystem.h"

namespace Immortal
{
namespace OpenGL
{

static constexpr size_t MAX_SHADER_SOURCE = 2;

Shader::Shader(const std::string &filepath, Type type) :
    Super{ type },
    name{ std::move(FileSystem::ExtractFileName(filepath)) }
{
    std::vector<GLuint> shaders;
    shaders.reserve(MAX_SHADER_SOURCE);
    if (type == Shader::Type::Graphics)
    {
        shaders.emplace_back(CompileShader(GL_VERTEX_SHADER,   FileSystem::ReadString(filepath + ".vert").c_str()));
        shaders.emplace_back(CompileShader(GL_FRAGMENT_SHADER, FileSystem::ReadString(filepath + ".frag").c_str()));
    }
    if (type == Type::Compute)
    {
        shaders.emplace_back(CompileShader(GL_COMPUTE_SHADER, FileSystem::ReadString(filepath + ".comp").c_str()));
    }

    Link(shaders);
}

Shader::~Shader()
{
    glDeleteProgram(handle);
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

std::string Shader::__InjectPreamble(const std::string &source)
{
#ifdef __APPLE__
#   define PLATFORM_STRING STR(__APPLE__)
#elif defined(_WIN32)
#   define PLATFORM_STRING STR(_WIN32)
#elif defined(__linux__)
#	define PLATFORM_STRING STR(__linux__)
#else
#	define PLATFORM_STRING "Unknown"
#endif

	static std::string preamble = "#define push_constant binding=" PUSH_CONSTANT_LOCATION_STR "\n#define " PLATFORM_STRING "\n";
	size_t index = source.find("layout");
	return source.substr(0, index) + preamble + source.substr(index);
}

uint32_t Shader::CompileShader(GLenum type, const std::string &source)
{
	std::string shaderSource = __InjectPreamble(source);
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

void Shader::Set(const std::string &name, int value)
{
    GLint location = glGetUniformLocation(handle, name.c_str());
    glUniform1i(location, value);
}

void Shader::Set(const std::string & name, int * values, uint32_t count)
{
    GLint location = glGetUniformLocation(handle, name.c_str());
    glUniform1iv(location, count, values);
}

void Shader::Set(const std::string & name, float value)
{
    GLint location = glGetUniformLocation(handle, name.c_str());
    glUniform1f(location, value);
}

void Shader::Set(const std::string & name, const Vector::Vector2 & value)
{
    GLint location = glGetUniformLocation(handle, name.c_str());
    glUniform2f(location, value.x, value.y);
}

void Shader::Set(const std::string & name, const Vector3 & value)
{
    GLint location = glGetUniformLocation(handle, name.c_str());
    glUniform3f(location, value.x, value.y, value.z);
}

void Shader::Set(const std::string & name, const Vector::Vector4 & value)
{
    GLint location = glGetUniformLocation(handle, name.c_str());
    glUniform4f(location, value.x, value.y, value.z, value.w);
}

void Shader::Set(const std::string &name, const Matrix4 & matrix)
{
    GLint location = glGetUniformLocation(handle, name.c_str());
    glUniformMatrix4fv(location, 1, GL_FALSE, &(matrix[0].x));
}

void Shader::DispatchCompute(uint32_t numGroupsX, uint32_t numGroupsY, uint32_t numGroupsZ)
{
    // SLASSERT(type == Shader::Type::Compute && "The shader type is not compute.");
    glDispatchCompute(numGroupsX, numGroupsY, numGroupsZ);
}

}
}
