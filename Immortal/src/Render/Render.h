#pragma once

#include "ImmortalCore.h"

#include "RendererAPI.h"


namespace Immortal
{

class Render
{
public:
    static void Init()
    {
        RendererAPI->Init();
    }

    static void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
    {
        RendererAPI->SetViewport(x, y, width, height);
    }

    static void SetClearColor(const Vector4 &color)
    {
        RendererAPI->SetClearColor(color);
    }

    static void EnableDepthTest()
    {
        RendererAPI->EnableDepthTest();
    }

    static void DisableDepthTest()
    {
        RendererAPI->DisableDepthTest();
    }

    static void Clear()
    {
        RendererAPI->Clear();
    }

    static void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t count = 0)
    {
        RendererAPI->DrawIndexed(vertexArray, count);
    }

private:
    static Scope<RendererAPI> RendererAPI;
};

}
