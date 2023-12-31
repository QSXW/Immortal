#pragma once

#include "Common.h"

#include "Algorithm/LightArray.h"
#include "Graphics/RenderTarget.h"
#include "Graphics/Pipeline.h"
#include "Graphics/Texture.h"
#include "Graphics/LightGraphics.h"
#include "VertexArray.h"
#include "Descriptor.h"

#include <vector>

namespace Immortal
{
namespace OpenGL
{

class Shader;
class IMMORTAL_API Pipeline : virtual public SuperGraphicsPipeline, virtual public SuperComputePipeline, public Handle
{
public:
    using Super = SuperGraphicsPipeline;

public:
	Pipeline();

    virtual ~Pipeline() override;

    virtual void Construct(SuperShader **ppShader, size_t shaderCount, const InputElementDescription &description, const std::vector<Format> &outputDescription);

public:
	void SetState();

public:
	VertexArray *GetVextexArray()
	{
		return &vertexArray;
	}

    uint32_t GetPushConstantIndex() const
    {
		return pushConstantIndex;
    }

    const std::vector<Descriptor> &GetDescriptors() const
    {
		return descriptors;
    }

protected:
    void LikeProgram(Shader **ppShader, size_t shaderCount);

    void Reflect();

protected:
    VertexArray vertexArray;

    uint32_t pushConstantIndex;

	std::vector<Descriptor> descriptors;
};

}
}
