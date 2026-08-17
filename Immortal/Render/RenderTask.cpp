#include "RenderTask.h"

namespace Immortal
{

RenderTask::RenderTask(const std::string &name, const Flags &flags) :
    IClass{name.c_str()},
    name{name},
    dependency{},
    flags{flags}
{

}

RenderTask::~RenderTask()
{

}

void RenderTask::SetDependency(const Ref<RenderTask> &task)
{
	dependency = task;
}

const Ref<RenderTask> &RenderTask::GetDependency() const
{
	return dependency;
}

const std::string &RenderTask::GetName() const
{
	return name;
}

void RenderTask::SetName(const std::string &value)
{
	name = value;
}

}
