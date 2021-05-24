#include "impch.h"
#include "VertexArray.h"

#include "Immortal/Platform/OpenGL/OpenGLVertexArray.h"

#include "Renderer.h"

namespace Immortal {

	Ref<VertexArray> VertexArray::Create()
	{
		return InstantiateGrphicsPrimitive<VertexArray, OpenGLVertexArray, OpenGLVertexArray, OpenGLVertexArray>();
	}

}