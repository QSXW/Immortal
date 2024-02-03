#pragma once

#include "Common.h"
#include "Handle.h"
#include "Graphics/Shader.h"
#include <string>

namespace Immortal
{
namespace Metal
{

class Device;
class IMMORTAL_API Shader : public SuperShader, public Handle<MTL::Library>
{
public:
    using Super     = SuperShader;
    METAL_SWAPPABLE(Shader)

public:
    Shader();

    Shader(Device *device, const std::string &name, Stage stage, const std::string &source, const std::string &entryPoint);

    virtual ~Shader() override;

    std::string GetMetalShaderSource(const std::string &source, const std::string &name, ShaderStage stage, const std::string &entryPoint);

public:
    MTL::Function *GetFunction() const
    {
        return function;
    }

    Stage GetStage() const
    {
        return stage;
    }

    const Size3D &GetThreadGroupSize() const
    {
        return threadGroupSize;
    }

    void Swap(Shader &other)
    {
        Handle::Swap(other);
        std::swap(function,        other.function       );
        std::swap(name,            other.name           );
        std::swap(stage,           other.stage          );
        std::swap(threadGroupSize, other.threadGroupSize);
    }

protected:
    MTL::Function *function;

    std::string name;

    Stage stage;

    Size3D threadGroupSize;
};

}
}
