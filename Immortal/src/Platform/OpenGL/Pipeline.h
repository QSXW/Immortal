#pragma once

#include "Common.h"

#include "Render/RenderTarget.h"
#include "Render/Pipeline.h"
#include "Render/Texture.h"
#include "Shader.h"
#include "Buffer.h"
#include "VertexArray.h"
#include "Buffer.h"

namespace Immortal
{
namespace OpenGL
{

class Pipeline : public SuperPipeline
{
public:
    using Super = SuperPipeline;

public:
    Pipeline(std::shared_ptr<Shader::Super> &shader);

    virtual ~Pipeline();

    virtual void Set(const InputElementDescription &description) override;

    virtual void Bind(std::shared_ptr<SuperTexture> &texture, uint32_t slot) override;

    template <Buffer::Type type>
    std::shared_ptr<Buffer> Get()
    {
        if constexpr (type == Buffer::Type::Vertex)
        {
            return std::dynamic_pointer_cast<Buffer>(desc.vertexBuffers[0]);
        }
        if constexpr (type == Buffer::Type::Index)
        {
            return std::dynamic_pointer_cast<Buffer>(desc.indexBuffer);
        }
    }

    virtual void Set(std::shared_ptr<SuperBuffer> &buffer) override;

private:
    VertexArray handle;

    InputElementDescription inputElementDesription;
};

}
}
