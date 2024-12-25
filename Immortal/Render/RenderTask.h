#pragma once

#include "Shared/IObject.h"
#include "Math/Vector.h"
#include "Graphics.h"

namespace Immortal
{

struct SceneParameters
{
	struct
	{
		Vector4 direction;
		Vector4 radiance;
	} lights[4];
	Matrix4 viewProjection;
	Matrix4 skyboxProjection;
	float exposure;
	float gamma;
};

class RenderTask : public IObject
{
public:
	RenderTask(const std::string &name);

	virtual ~RenderTask();

    virtual void Build(AsyncComputeThread *asyncComputeThread) = 0;

	virtual void Execute(CommandBuffer *commandBuffer, const SceneParameters &params) = 0;

    virtual void Composite(CommandBuffer *commandBuffer, const SceneParameters &params) = 0;

public:
    void SetDependency(const Ref<RenderTask> &task);

    const Ref<RenderTask> &GetDependency() const;

    const std::string &GetName() const;

    void SetName(const std::string &value);

protected:
    std::string name;

    Ref<RenderTask> dependency;
};

}
