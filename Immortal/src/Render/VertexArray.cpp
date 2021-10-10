#include "impch.h"
#include "VertexArray.h"

#include "Platform/OpenGL/VertexArray.h"
#include "Render.h"

namespace Immortal
{

std::shared_ptr<VertexArray> VertexArray::Create()
{
    return CreateSuper<VertexArray, OpenGL::VertexArray, OpenGL::VertexArray, OpenGL::VertexArray>();
}

}
