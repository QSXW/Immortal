#include "impch.h"
#include "Shader.h"

#include <fstream>
#include <array>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace Immortal
{
namespace OpenGL
{

static GLenum ShaderTypeFromString(const std::string &type)
{
    if (type == "vertex")
    {
        return GL_VERTEX_SHADER;
    }
    else if (type == "fragment" || type == "pixel")
    {
        return GL_FRAGMENT_SHADER;
    }
    else if (type == "compute")
    {
        return GL_COMPUTE_SHADER;
    }

    SLASSERT(false && "Unknown shader type!");
    return 0;
}

Shader::Shader(const std::string &filepath)
{
    auto source = ReadFile(filepath);
    auto shaderSources = Preprocess(source);
        
    if (shaderSources.find(GL_COMPUTE_SHADER) != shaderSources.end())
    {
        type = Shader::Type::Compute;
    }

    Compile(shaderSources);

    /* Extract name from filepath */
    auto lastSlash = filepath.find_last_of("/\\");
    lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
    auto lastDot = filepath.rfind('.');
    auto count = lastDot == std::string::npos ? filepath.size() - lastSlash : lastDot - lastSlash;
    name = filepath.substr(lastSlash, count);
}

Shader::Shader(const std::string &name, const std::string & vertexSrc, const std::string & fragmentSrc)
    : name(name)
{
    std::unordered_map<GLenum, std::string> sources;
    sources[GL_VERTEX_SHADER] = vertexSrc;
    sources[GL_FRAGMENT_SHADER] = fragmentSrc;
    Compile(sources);
}

Shader::~Shader()
{
    glDeleteProgram(handle);
}

void Shader::Compile(const std::unordered_map<GLenum, std::string>& source)
{
    std::array<GLenum, 2> shaderIDs{};

    int index = 0;
    handle = glCreateProgram();
    for (auto &src : source)
    {
        shaderIDs[index] = CompileShader(src.first, src.second.c_str());
        glAttachShader(handle, shaderIDs[index]);
        index++;
    }
    glLinkProgram(handle);

    GLint isLinked = 0;
    glGetProgramiv(handle, GL_LINK_STATUS, &isLinked);
    if (isLinked == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &isLinked);

        std::unique_ptr<GLchar> infoLog = std::make_unique<GLchar>(maxLength);
        glGetProgramInfoLog(handle, maxLength, &maxLength, infoLog.get());

        for (int i = 0; i < source.size(); i++)
        {
            glDeleteShader(shaderIDs[i]);
        }

        LOG::ERR("{0}", infoLog.get());
        SLASSERT(isLinked == GL_FALSE && "Shader compilation failure!");
    }

    for (int i = 0; i < source.size(); i++)
    {
        glDetachShader(handle, shaderIDs[i]);
    }	
}

uint32_t Shader::CompileShader(int type, const char *src)
{
    uint32_t shader = glCreateShader(type);

    glShaderSource(shader, 1, &src, 0);
    glCompileShader(shader);

    GLint isCompiled = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
    if (isCompiled == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

        std::vector<GLchar> error;
        error.resize(maxLength);
        glGetShaderInfoLog(shader, maxLength, &maxLength, error.data());
        glDeleteShader(shader);
        LOG::ERR("{0}", error.data());
        SLASSERT(isCompiled && "Shader compilation failure!");
    }

    return shader;
}

std::string Shader::ReadFile(const std::string & filepath)
{
    std::string result;
    std::ifstream in(filepath, std::ios::in | std::ios::binary); // ifstream closes itself due to RAII
    if (in)
    {
        in.seekg(0, std::ios::end);
        size_t size = in.tellg();
        if (size != -1)
        {
            result.resize(size);
            in.seekg(0, std::ios::beg);
            in.read(&result[0], size);
            return result;
        }
    }
    LOG::ERR("Could not open file '{0}'", filepath);
    return {};
}

std::unordered_map<GLenum, std::string> Shader::Preprocess(const std::string & source)
{
    std::unordered_map<GLenum, std::string> shaderSources;

    const char* typeToken = "#type";
    size_t typeTokenLength = strlen(typeToken);
    size_t pos = source.find(typeToken, 0); //Start of shader type declaration line
    while (pos != std::string::npos)
    {
        size_t eol = source.find_first_of("\r\n", pos); //End of shader type declaration line
        SLASSERT(eol != std::string::npos && "Syntax error");
        size_t begin = pos + typeTokenLength + 1; //Start of shader type name (after "#type " keyword)
        std::string type = source.substr(begin, eol - begin);
        SLASSERT(ShaderTypeFromString(type) && "Invalid shader type specified");

        size_t nextLinePos = source.find_first_not_of("\r\n", eol); //Start of shader code after shader type declaration line
        SLASSERT(nextLinePos != std::string::npos && "Syntax error");
        pos = source.find(typeToken, nextLinePos); //Start of next shader type declaration line

        shaderSources[ShaderTypeFromString(type)] = (pos == std::string::npos) ? source.substr(nextLinePos) : source.substr(nextLinePos, pos - nextLinePos);
    }

    return shaderSources;
}

void Shader::Map() const
{
    glUseProgram(handle);
}

void Shader::Unmap() const
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

void Shader::Set(const std::string & name, const Vector::Vector3 & value)
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
    SLASSERT(type == Shader::Type::Compute && "The shader type is not compute.");
    glDispatchCompute(numGroupsX, numGroupsY, numGroupsZ);
}

}
}
