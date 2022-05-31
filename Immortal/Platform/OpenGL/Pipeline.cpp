#include "Pipeline.h"

namespace Immortal
{
namespace OpenGL
{

Pipeline::Pipeline(Ref<Shader::Super> shader) :
    Super{ shader },
    handle{ }
{

}

Pipeline::~Pipeline()
{

}

void Pipeline::Set(const InputElementDescription &description)
{
    if (!desc.vertexBuffers.empty())
    {
        handle.Set(dynamic_cast<Buffer *>(desc.vertexBuffers[0].Get()), description);
    }
    else
    {
        inputElementDesription = description;
    }
}

void Pipeline::Bind(const std::string &name, const Buffer::Super *superUniform)
{

}

void Pipeline::Bind(Texture *texture, uint32_t slot)
{

}

void Pipeline::Set(Ref<SuperBuffer> buffer)
{
    if (buffer->GetType() == Buffer::Type::Vertex)
    {
        desc.vertexBuffers.emplace_back(buffer);
        if (!inputElementDesription.Empty())
        {
            handle.Set(dynamic_cast<Buffer *>(desc.vertexBuffers[0].Get()), inputElementDesription);
        }
    }
    if (buffer->GetType() == Buffer::Type::Index)
    {
        desc.indexBuffer = buffer;
    }
    handle.Bind(dynamic_cast<Buffer *>(buffer.Get()));
}

void Pipeline::Bind(const Descriptor::Super *descriptors, uint32_t slot)
{
    auto textures = rcast<const GLuint *>(descriptors);

    for (int i = 0; i < 32; i++)
    {
        glBindTextureUnit(i, textures[i]);
    }
}

}
}
