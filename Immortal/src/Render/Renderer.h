#pragma once

#include "ImmortalCore.h"

#include "RenderContext.h"
#include "VertexArray.h"

namespace Immortal
{

class IMMORTAL_API Renderer
{
public:
    virtual void INIT() { }

    virtual void SetViewport(UINT32 x, UINT32 y, UINT32 width, UINT32 height) { }

    virtual void EnableDepthTest() { }

    virtual void DisableDepthTest() { }

    virtual void SetClearColor(const Vector4 &color) { }

    virtual void Clear() { }

    virtual void DrawIndexed(const Ref<VertexArray> &vertexArray, UINT32 indexCount) { }

    virtual void SwapBuffers() { }

    virtual void RenderFrame() { }

    virtual uint32_t Index() { return 0; }

    static std::unique_ptr<Renderer> Create(RenderContext *context);
};

using SuperRenderer = Renderer;

}
