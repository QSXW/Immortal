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
    handle.Set(description);
}

void Pipeline::Bind(std::shared_ptr<SuperTexture> &texture, uint32_t slot)
{

}

}
}
