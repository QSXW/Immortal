#include "Pipeline.h"

namespace Immortal
{
namespace OpenGL
{

Pipeline::Pipeline(std::shared_ptr<Shader::Super> &shader) :
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
        handle.Set(std::dynamic_pointer_cast<Buffer>(desc.vertexBuffers[0]).get(), description);
    }
    else
    {
        inputElementDesription = description;
    }
}

void Pipeline::Bind(const std::string &name, const Buffer::Super *superUniform)
{

}

void Pipeline::Bind(const std::shared_ptr<SuperTexture> &texture, uint32_t slot)
{

}

void Pipeline::Set(std::shared_ptr<SuperBuffer> &buffer)
{
    if (buffer->GetType() == Buffer::Type::Vertex)
    {
        desc.vertexBuffers.emplace_back(buffer);
        if (!inputElementDesription.Empty())
        {
            handle.Set(std::dynamic_pointer_cast<Buffer>(desc.vertexBuffers[0]).get(), inputElementDesription);
        }
    }
    if (buffer->GetType() == Buffer::Type::Index)
    {
        desc.indexBuffer = buffer;
    }
    handle.Bind(std::dynamic_pointer_cast<Buffer>(buffer).get());
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
