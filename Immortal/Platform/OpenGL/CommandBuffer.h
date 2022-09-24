#pragma once

#include <queue>
#include <functional>
#include <string>

namespace Immortal
{
namespace OpenGL
{

class CommandBuffer 
{
public:
	CommandBuffer(const std::string &name = "OpenGL::CommandBuffer");

	void Execute();

	template <class T>
	void Submit(T &&command)
	{
		recorder.push(command);
	}

protected:
	std::string name;

	std::queue<std::function<void()>> recorder;
};

}
}
