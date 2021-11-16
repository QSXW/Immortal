#pragma once

#include "ImmortalCore.h"

#include "RenderContext.h"
#include "VertexArray.h"
#include "Texture.h"
#include "Pipeline.h"
#include "Framebuffer.h"

namespace Immortal
{

class IMMORTAL_API Renderer
{
public:
    virtual ~Renderer() { }

    virtual void INIT() { }

    virtual void OnResize(UINT32 x, UINT32 y, UINT32 width, UINT32 height) { }

    virtual void EnableDepthTest() { }

    virtual void DisableDepthTest() { }

    virtual void SetClearColor(const Vector4 &color) { }

    virtual void Clear() { }

    virtual void DrawIndexed(const std::shared_ptr<VertexArray> &vertexArray, UINT32 indexCount) { }

    virtual void SwapBuffers() { }

    virtual void PrepareFrame() { }

    virtual uint32_t Index() { return 0; }

    static std::unique_ptr<Renderer> Create(RenderContext *context);

    virtual std::shared_ptr<Shader> CreateShader(const std::string &filepath, Shader::Type type)
    {
        return nullptr;
    }

    virtual std::shared_ptr<Pipeline> CreatePipeline(std::shared_ptr<Shader> &shader)
    {
        return nullptr;
    }

    virtual const char *GraphicsRenderer()
    {
        return "None";
    };

    virtual std::shared_ptr<Texture> CreateTexture(const std::string &filepath)
    {
        return nullptr;
    }

    virtual std::shared_ptr<SuperTexture> CreateTexture(uint32_t width, uint32_t height, const void *data, const Texture::Description &description)
    {
        return nullptr;
    }

    virtual std::shared_ptr<Buffer> CreateBuffer(const size_t size, const void *data, Buffer::Type type)
    {
        return nullptr;
    }

    virtual std::shared_ptr<Buffer> CreateBuffer(const size_t size, Buffer::Type type)
    {
        return nullptr;
    }

    virtual std::shared_ptr <Framebuffer> CreateFramebuffer(const Framebuffer::Description &description)
    {
        return nullptr;
    }

    virtual void Draw(const std::shared_ptr<Pipeline> &pipeline)
    {
    
    }
};

using SuperRenderer = Renderer;

}
