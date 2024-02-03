#include "Pipeline.h"
#include "Shader.h"
#include "Buffer.h"
#include "Shared/Log.h"

namespace Immortal
{
namespace OpenGL
{

Pipeline::Pipeline() :
    Super{},
    Handle{},
    pushConstantIndex{}
{
}

Pipeline::~Pipeline()
{
}

void Pipeline::Construct(SuperShader **_ppShader, size_t shaderCount, const InputElementDescription &description, const std::vector<Format> &outputDescription)
{
    Shader **ppShader = (Shader **) _ppShader;
    LikeProgram(ppShader, shaderCount);
    Reflect();

    vertexArray = VertexArray{ description };
}


void Pipeline::SetState()
{
    glEnable(GL_SCISSOR_TEST);
    if (flags & State::Blend)
    {
        glEnable(GL_BLEND);
        glBlendEquation(GL_FUNC_ADD);
        glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    }
    if (flags & State::Depth)
    {
        glEnable(GL_DEPTH_TEST);
    }
    else
    {
        glDisable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_STENCIL_TEST);
    }
}

void Pipeline::LikeProgram(Shader **ppShader, size_t shaderCount)
{
    handle = glCreateProgram();

    for (size_t i = 0; i < shaderCount; i++)
    {
        glAttachShader(handle, *ppShader[i]);
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

        LOG::ERR("Program link failure!");
        LOG::ERR("{}", error.data());
    }

    for (size_t i = 0; i < shaderCount; i++)
    {
        glDetachShader(handle, *ppShader[i]);
    }
}

void Pipeline::Reflect()
{
    GLsizei length;
    GLint size;
    GLenum type;
    GLchar name[256] = {};

    GLint numUniform;
    glGetProgramiv(handle, GL_ACTIVE_UNIFORMS, &numUniform);

    uint32_t bufferIndex = 0;

    for (int i = 0; i < numUniform; i++)
    {
        glGetActiveUniform(handle, i, sizeof(name), &length, &size, &type, name);
        if (std::string{name}.find("push_constant") != std::string::npos ||
            std::string{name}.find("PushConstant") != std::string::npos)
        {
            pushConstantIndex = i;
            continue;
        }

        DescriptorType descriptorType = DescriptorType::UniformBuffer;
        if (type == GL_SAMPLER_2D || type == GL_IMAGE_2D)
        {
            descriptorType = type == GL_IMAGE_2D ? DescriptorType::Image2D : DescriptorType::SamplerImage2D;
            descriptors.emplace_back(Descriptor{
                .handle  = 0,
                .binding = (uint32_t)i,
                .type    = descriptorType
            });
        }

    }
}

}
}
