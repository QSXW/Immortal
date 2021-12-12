#pragma once

#include "ImmortalCore.h"

#include "RenderContext.h"
#include "Texture.h"
#include "Pipeline.h"
#include "RenderTarget.h"
#include "Descriptor.h"

namespace Immortal
{

class IMMORTAL_API Renderer
{
public:
    virtual ~Renderer() { }

    virtual void Setup() { }

    virtual void OnResize(UINT32 x, UINT32 y, UINT32 width, UINT32 height) { }

    virtual void EnableDepthTest() { }

    virtual void DisableDepthTest() { }

    virtual void SetClearColor(const Vector4 &color) { }

    virtual void Clear() { }

    virtual void SwapBuffers() { }

    virtual void PrepareFrame() { }

    virtual uint32_t Index() { return 0; }

    virtual const char *GraphicsRenderer()
    {
        return "None";
    };

    static std::unique_ptr<Renderer> Create(RenderContext *context);

    virtual std::shared_ptr<Shader> CreateShader(const std::string &filepath, Shader::Type type)
    {
        return nullptr;
    }

    virtual std::shared_ptr<Pipeline> CreatePipeline(std::shared_ptr<Shader> &shader)
    {
        return nullptr;
    }

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

    virtual std::shared_ptr<Buffer> CreateBuffer(const size_t size, int binding)
    {
        return nullptr;
    }

    virtual std::shared_ptr<RenderTarget>CreateRenderTarget(const RenderTarget::Description &description)
    {
        return nullptr;
    }

    virtual void Draw(const std::shared_ptr<Pipeline> &pipeline)
    {
    
    }

    virtual void Begin(std::shared_ptr<RenderTarget> &renderTarget)
    {

    }

    virtual void End()
    {

    }

    virtual Descriptor *CreateImageDescriptor(uint32_t count)
    {
        return nullptr;
    }

    virtual Descriptor *CreateBufferDescriptor(uint32_t count)
    {
        return nullptr;
    }
};

using SuperRenderer = Renderer;

}
