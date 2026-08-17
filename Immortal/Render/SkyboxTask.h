#pragma once

#include "RenderTask.h"
#include "Shared/IObject.h"
#include "Graphics.h"
#include "Mesh.h"
#include "String/IString.h"

namespace Immortal
{

class SkyboxTask : public RenderTask
{
public:
	SkyboxTask();

    virtual ~SkyboxTask() override;

    virtual void Build(AsyncComputeThread *asyncComputeThread) override;

    virtual void Execute(CommandBuffer *commandBuffer, const SceneParameters &params) override;

    virtual void Composite(CommandBuffer *commandBuffer, const SceneParameters &params) override;

	void OnFrameGraphDebugGui() override;

    void SetFilePath(const String &value);

	const Ref<Texture> &GetRadianceCubemap() const
	{
		return textureCube;
	}

protected:
	String filepath;

    Ref<Texture> texture;

    Ref<Texture> dstTexture;

    Ref<Buffer> buffer;

    Ref<DescriptorSet> descriptorSet;

	Ref<Texture> textureCube;

    Ref<Sampler> sampler;

    Ref<Pipeline> pipeline;

    Ref<GraphicsPipeline> graphicsPipeline;

    Ref<DescriptorSet> skyboxDescriptorSet;

    Ref<Mesh> skybox;

    Ref<Buffer> vertexBuffer;

    Ref<Buffer> indexBuffer;
};

}
