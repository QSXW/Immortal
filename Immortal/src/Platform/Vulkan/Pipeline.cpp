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

void Pipeline::Set(std::shared_ptr<Buffer> &buffer, Buffer::Type type)
{
    if (type == Buffer::Type::Vertex)
    {
        INITVertex();
    }
}

void Pipeline::Set(const InputElementDescription &description)
{
    auto size                       = description.Size();
    auto &inputAttributeDescription = configuration->inputAttributeDescriptions;

    inputAttributeDescription.resize(size);
    for (int i = 0; i < size; i++)
    {
        inputAttributeDescription[i].binding  = 0;
        inputAttributeDescription[i].location = i;
        inputAttributeDescription[i].format   = description[i].BaseType<VkFormat>();
        inputAttributeDescription[i].offset   = description[i].Offset();
    }

    configuration->vertexInputBidings.emplace_back(VkVertexInputBindingDescription{
            0,
            description.Stride(),
            VK_VERTEX_INPUT_RATE_VERTEX
        });

    INITLayout();
}

}
}
