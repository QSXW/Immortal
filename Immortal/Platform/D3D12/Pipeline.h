#pragma once

#include "Common.h"
#include "Render/RenderTarget.h"
#include "Render/Pipeline.h"
#include "Render/Texture.h"
#include "Shader.h"

namespace Immortal
{
namespace D3D12
{

class Device;
class Pipeline : public SuperPipeline
{
public:
    using Super = SuperPipeline;

public:
    Pipeline(Device *device, std::shared_ptr<Shader::Super> shader);

    virtual ~Pipeline();

private:
    ID3D12PipelineState *pipelineState{ nullptr };
};

}
}
