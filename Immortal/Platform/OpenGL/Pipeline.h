#pragma once

#include "Common.h"

#include "Algorithm/LightArray.h"
#include "Render/RenderTarget.h"
#include "Render/Pipeline.h"
#include "Render/Texture.h"
#include "Shader.h"
#include "Buffer.h"
#include "VertexArray.h"
#include "Buffer.h"
#include "Descriptor.h"
#include "Texture.h"

#include <vector>

namespace Immortal
{
namespace OpenGL
{

class Pipeline : virtual public SuperGraphicsPipeline, virtual public ComputePipeline
{
public:
    using Super = SuperGraphicsPipeline;

public:
    Pipeline(Ref<Shader::Super> shader);

    virtual ~Pipeline();

    virtual void Set(const InputElementDescription &description) override;

    virtual void Set(Ref<SuperBuffer> buffer) override;

    virtual void Bind(const std::string &name, const SuperBuffer *uniform) override;

    virtual void Bind(SuperBuffer *buffer, uint32_t binding = 0) override;

    virtual void Bind(Texture::Super *texture, uint32_t binding = 0) override;

    virtual void Bind(const DescriptorBuffer *descriptorBuffer, uint32_t binding) override;

    virtual Anonymous AllocateDescriptorSet(uint64_t uuid) override;

    virtual void Dispatch(uint32_t nGroupX, uint32_t nGroupY, uint32_t nGroupZ = 0) override;

    virtual void PushConstant(uint32_t size, const void *data, uint32_t offset = 0) override;

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

    void Draw();

private:
	void __BindDescriptorTable() const;

private:
    VertexArray handle;

    Shader *shader;

    LightArray<Descriptor> bufferDescriptorTable;

    LightArray<Descriptor, 32> imageDescriptorTable;

    InputElementDescription inputElementDesription;

    URef<UniformBuffer> pushConstants;
};

}
}
