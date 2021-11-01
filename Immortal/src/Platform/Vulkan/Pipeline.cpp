#include "Pipeline.h"

namespace Immortal
{
namespace Vulkan
{

Pipeline::Pipeline(Device *device, std::shared_ptr<Shader::Super> &shader) :
    device{ device },
    Super{ shader }
{
    configuration = std::make_unique<Configuration>();
    CleanUpObject(configuration.get());
}

Pipeline::~Pipeline()
{

}

void Pipeline::Set(std::shared_ptr<Buffer> &buffer)
{
    if (buffer->GetType() == Buffer::Type::Vertex)
    {
        desc.vertexBuffers.emplace_back(buffer);
        INITVertex();
    }
    if (buffer->GetType() == Buffer::Type::Index)
    {
        desc.indexBuffer = buffer;
    }
}

void Pipeline::Set(const InputElementDescription &description)
{
    Super::Set(std::move(description));
    auto size                       = desc.Layout.Size();
    auto &inputAttributeDescription = configuration->inputAttributeDescriptions;

    inputAttributeDescription.resize(size);
    for (int i = 0; i < size; i++)
    {
        inputAttributeDescription[i].binding  = 0;
        inputAttributeDescription[i].location = i;
        inputAttributeDescription[i].format   = desc.Layout[i].BaseType<VkFormat>();
        inputAttributeDescription[i].offset   = desc.Layout[i].Offset();
    }

    configuration->vertexInputBidings.emplace_back(VkVertexInputBindingDescription{
            0,
            desc.Layout.Stride(),
            VK_VERTEX_INPUT_RATE_VERTEX
        });

    INITLayout();
}

}
}
