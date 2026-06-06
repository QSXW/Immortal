#pragma once



#include "Graphics.h"

#include "Graphics/RenderTarget.h"

#include "Shared/IObject.h"

#include "Math/Vector.h"



namespace Immortal

{



class EditorGridTask : public IObject

{

public:

	EditorGridTask();



	~EditorGridTask();



	void Build(AsyncComputeThread *asyncComputeThread);



	/** Optional: blend grid onto an active color+depth RT (depth test off in pipeline). */

	void ExecuteUnderlay(CommandBuffer *commandBuffer, const Matrix4 &invViewProjection, const Matrix4 &viewProjection, const Vector4 &cameraWorld);



	/** Fullscreen composite: sceneColor + depthTexture -> postOutputTarget (RGBA8). */

	void ExecutePost(CommandBuffer *commandBuffer, Texture *sceneColor, Texture *depthTexture, const Matrix4 &invViewProjection, const Matrix4 &viewProjection, const Vector4 &cameraWorld);



	bool IsReady() const

	{

		return built && underlayPipeline && postPipeline && postDescriptorSet && underlayDescriptorSet;

	}



	void SetViewportSize(const Vector2 &size);



	Ref<RenderTarget> GetPostOutputTarget() const

	{

		return postOutputTarget;

	}



private:

	Ref<GraphicsPipeline> underlayPipeline;



	Ref<GraphicsPipeline> postPipeline;



	Ref<DescriptorSet> underlayDescriptorSet;



	Ref<DescriptorSet> postDescriptorSet;



	Ref<Sampler> linearSampler;



	Ref<Texture> bindGuardTexture;



	Ref<RenderTarget> postOutputTarget;



	bool built = false;

};



}


