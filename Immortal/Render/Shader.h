#pragma once

#include "Core.h"
#include "Format.h"
#include "Buffer.h"

namespace Immortal
{

class IMMORTAL_API Shader : public IObject
{
public:
    using Stage = ShaderStage;

public:
    virtual ~Shader() = default;
};

using SuperShader = Shader;

namespace Interface
{
    using Shader = SuperShader;
}

}
