#pragma once

#include "ImmortalCore.h"

#include "RendererAPI.h"


namespace Immortal
{

class RenderCommand
{
public:
    static void Init()
    {
        sRendererAPI->Init();
    }

    static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
    {
        sRendererAPI->SetViewport(x, y, width, height);
    }

    static void SetClearColor(const Vector::Vector4 &color)
    {
        sRendererAPI->SetClearColor(color);
    }
        
    static void EnableDepthTest()
    {
        sRendererAPI->EnableDepthTest();
    }

    static void DisableDepthTest()
    {
        sRendererAPI->DisableDepthTest();
    }

    static void Clear()
    {
        sRendererAPI->Clear();
    }

    static void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t count = 0)
    {
        sRendererAPI->DrawIndexed(vertexArray, count);
    }

private:
    static Scope<RendererAPI> sRendererAPI;
};

}

