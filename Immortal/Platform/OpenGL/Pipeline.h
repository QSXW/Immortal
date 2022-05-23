#pragma once

#include "Common.h"

#include "Render/RenderTarget.h"
#include "Render/Pipeline.h"
#include "Render/Texture.h"
#include "Shader.h"
#include "Buffer.h"
#include "VertexArray.h"
#include "Buffer.h"
#include "Descriptor.h"

namespace Immortal
{
namespace OpenGL
{

class Pipeline : public SuperGraphicsPipeline
{
public:
    using Super = SuperGraphicsPipeline;

public:
    Pipeline(std::shared_ptr<Shader::Super> &shader);

    virtual ~Pipeline();

    virtual void Set(const InputElementDescription &description) override;

    virtual void Set(Ref<SuperBuffer> buffer) override;

    virtual void Bind(const std::string &name, const Buffer::Super *uniform) override;

    virtual void Bind(Texture *texture, uint32_t slot);

    virtual void Bind(const Descriptor::Super *descriptors, uint32_t slot) override;

    template <Buffer::Type type>
    Ref<Buffer> Get()
    {
        if constexpr (type == Buffer::Type::Vertex)
        {
            return dynamic_cast<Buffer *>(desc.vertexBuffers[0].Get());
        }
        if constexpr (type == Buffer::Type::Index)
        {
            return dynamic_cast<Buffer *>(desc.indexBuffer.Get());
        }
    }

    void Draw()
    {
        auto shader = std::dynamic_pointer_cast<Shader>(desc.shader);

        shader->Map();
        handle.Bind();

        auto vertexBuffer = Get<Buffer::Type::Vertex>();
        auto indexBuffer = Get<Buffer::Type::Index>();

        vertexBuffer->Bind();
        indexBuffer->Bind();

        glDrawElements(GL_TRIANGLES, ElementCount, GL_UNSIGNED_INT, 0);

        handle.Unbind();
        shader->Unmap();
    }

private:
    VertexArray handle;

    InputElementDescription inputElementDesription;
};

}
}
