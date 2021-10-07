#pragma once

#include "ImmortalCore.h"
#include "Render/Renderer.h"

#include "RenderContext.h"
#include "Render/VertexArray.h"
#include <glad/glad.h>

#include "Texture.h"

namespace Immortal
{
namespace OpenGL
{

class Renderer : public SuperRenderer
{
public:
    using Super = SuperRenderer;

public:
    Renderer(SuperRenderContext *context);

    virtual void INIT() override;

    virtual void OnResize(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;

    virtual void SetClearColor(const Vector::Vector4 &color) override;

    virtual void Clear() override;

    virtual void EnableDepthTest() override;

    virtual void DisableDepthTest() override;

    virtual void DrawIndexed(const std::shared_ptr<VertexArray> &vertexArray, uint32_t indexCount = 0) override;

    virtual void SwapBuffers() override;

    virtual const char *GraphicsRenderer() override
    {
        return context->GraphicsRenderer();
    }

    virtual std::shared_ptr<SuperTexture> CreateTexture(const std::string &filepath) override
    {
        return std::make_shared<Texture2D>(filepath);
    }

private:
    RenderContext *context{ nullptr };
};

}
}