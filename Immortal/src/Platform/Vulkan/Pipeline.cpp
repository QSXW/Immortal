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

}
}
