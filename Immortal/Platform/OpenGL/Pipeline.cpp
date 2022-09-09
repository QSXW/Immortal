#include "Pipeline.h"

namespace Immortal
{
namespace OpenGL
{

Pipeline::Pipeline(Ref<Shader::Super> shader) :
    Super{ shader },
    handle{},
    shader{ shader.InterpretAs<Shader>() },
    pushConstants{ new UniformBuffer{128, 0} }
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

void Pipeline::Bind(const std::string &name, const SuperBuffer *superUniform)
{

}

void Pipeline::Bind(SuperBuffer *super, uint32_t binding)
{
	auto buffer = dynamic_cast<Buffer *>(super);
	descriptorTable.emplace_back(Descriptor{ buffer->Handle(), DescriptorType::Buffer, binding });
}

void Pipeline::Bind(SuperTexture *super, uint32_t binding)
{
	Texture *texture = dynamic_cast<Texture *>(super);
	DescriptorType type;

    if (binding > 0)
    {
		type = DescriptorType::ImageWrite;
    }
    else
    {
		type = DescriptorType::Image;
    }

    descriptorTable.emplace_back(Descriptor{*texture, type, binding});
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

void Pipeline::Bind(const DescriptorBuffer *descriptorBuffer, uint32_t binding)
{
    for (int i = 0; i < descriptorBuffer->size(); i++)
    {
		descriptorTable.emplace_back(Descriptor{ *descriptorBuffer->DeRef<uint32_t>(i), DescriptorType::Image, i + binding });
    }
}

void Pipeline::Draw()
{
	shader->Activate();
	handle.Bind();

	auto vertexBuffer = Get<Buffer::Type::Vertex>();
	auto indexBuffer  = Get<Buffer::Type::Index>();

	vertexBuffer->Bind();
	indexBuffer->Bind();

    __BindDescriptorTable();
	glDrawElements(GL_TRIANGLES, ElementCount, GL_UNSIGNED_INT, 0);

	handle.Unbind();
	shader->Deactivate();
}

Anonymous Pipeline::AllocateDescriptorSet(uint64_t uuid)
{
	(void) uuid;
	shader->Activate();
    descriptorTable.clear();

	return nullptr;
}

void Pipeline::Dispatch(uint32_t nGroupX, uint32_t nGroupY, uint32_t nGroupZ)
{
	glBindBufferBase(GL_UNIFORM_BUFFER, PUSH_CONSTANT_LOCATION, *pushConstants);
	__BindDescriptorTable();
	shader->DispatchCompute(nGroupX, nGroupY, nGroupZ);
	shader->Deactivate();
}

void Pipeline::PushConstant(uint32_t size, const void *data, uint32_t offset)
{
	pushConstants->Update(size, data, offset);
}

void Pipeline::__BindDescriptorTable() const
{
	for (auto &descriptor : descriptorTable)
	{
		if (descriptor.Type == DescriptorType::Buffer)
		{
			glBindBufferBase(GL_UNIFORM_BUFFER, descriptor.Binding, descriptor);
		}
		else if (descriptor.Type == DescriptorType::ImageWrite)
		{
			glBindImageTexture(descriptor.Binding, descriptor, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
		}
        else
        {
			glBindTextureUnit(descriptor.Binding, descriptor);
        }
	}
}

}
}
