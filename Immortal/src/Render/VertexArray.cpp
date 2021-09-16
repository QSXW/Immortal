#include "impch.h"
#include "VertexArray.h"

#include "Platform/OpenGL/OpenGLVertexArray.h"

#include "Render.h"

namespace Immortal
{

Ref<VertexArray> VertexArray::Create()
{
    return CreateSuper<VertexArray, OpenGLVertexArray, OpenGLVertexArray, OpenGLVertexArray>();
}

}
