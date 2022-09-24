#include "CommandBuffer.h"

namespace Immortal
{
namespace OpenGL
{

CommandBuffer::CommandBuffer(const std::string &name) :
    name{name}
{
}

void CommandBuffer::Execute()
{
	while (!recorder.empty())
	{
		auto &command = recorder.front();
		command();
		recorder.pop();
	}
}

}
}
