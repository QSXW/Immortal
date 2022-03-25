#pragma once

#include "Core.h"
#include "Render/Renderer.h"

#include "RenderContext.h"
#include <glad/glad.h>

#include "Texture.h"
#include "Buffer.h"
#include "Shader.h"
#include "Pipeline.h"
#include "RenderTarget.h"

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

    virtual void Setup() override;

    virtual void OnResize(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;

    virtual void SetClearColor(const Vector::Vector4 &color) override;

    virtual void Clear() override;

    virtual void SwapBuffers() override;

    virtual const char *GraphicsRenderer() override
    {
        return context->GraphicsRenderer();
    }

    virtual Texture::Super *CreateTexture(const std::string &filepath, const Texture::Description &description = {}) override
    {
        return new Texture{ filepath, description };
    }

    virtual Texture::Super *CreateTexture(uint32_t width, uint32_t height, const void *data, const Texture::Description &description) override
    {
        return new Texture{ width, height, data, description, 0 };
    }

    virtual Buffer::Super *CreateBuffer(const size_t size, uint32_t binding) override
    {
        return new UniformBuffer{ U32(size), binding };
    }

    virtual Buffer::Super *CreateBuffer(const size_t size, const void *data, Buffer::Type type) override
    {
        return new Buffer{ U32(size), data, type };
    }

    virtual Buffer::Super *CreateBuffer(const size_t size, Buffer::Type type) override
    {
        return new Buffer{ U32(size), type };
    }

    virtual Pipeline::Super *CreateGraphicsPipeline(std::shared_ptr<SuperShader> shader)
    {
        return new Pipeline{ shader };
    }

    virtual RenderTarget::Super *CreateRenderTarget(const RenderTarget::Description &description) override
    {
        return new RenderTarget{ description };
    }

    virtual Shader::Super *CreateShader(const std::string &filepath, Shader::Type type) override
    {
        return new Shader{ filepath, type };
    }

    virtual Descriptor::Super *CreateImageDescriptor(uint32_t count) override;

    virtual void Draw(Pipeline::Super *pipeline) override;

    virtual void Begin(std::shared_ptr<RenderTarget::Super> &renderTarget) override;

    virtual void End() override;

private:
    RenderContext *context{ nullptr };
};

}
}
