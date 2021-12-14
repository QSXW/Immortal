#include "Pipeline.h"
#include "Device.h"

namespace Immortal
{
namespace D3D12
{

Pipeline::Pipeline(Device *device, std::shared_ptr<Shader::Super> shader) :
    Super{ shader }
{

}

Pipeline::~Pipeline()
{
    IfNotNullThenRelease(pipelineState);
}

}
}
