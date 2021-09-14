#pragma once

#include "ImmortalCore.h"

#include "VertexArray.h"

namespace Immortal
{
class IMMORTAL_API RendererAPI
{
public:
    enum class Type
    {
        None = 0,
        OpenGL,
        D3D12,
        VulKan
    };

public:
    virtual ~RendererAPI() = default;

    virtual void Init() = 0;

    virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {};

    virtual void SetClearColor(const Vector::Color &color) {};

    virtual void Clear() {};

    virtual void EnableDepthTest() {};

    virtual void DisableDepthTest() {};

    virtual void DrawIndexed(const Ref<VertexArray>&vertexArray, uint32_t indexCount = 0) {};

    static Type GetAPI() { return API; }

    static void SetAPI(RendererAPI::Type &&api)
    {
        API = api;
    }
    static Scope<RendererAPI> Create();

public:
    static RendererAPI::Type API;
};
}

