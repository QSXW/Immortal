#pragma once

#include "Core.h"
#include "Render/RenderContext.h"
#include "Pipeline.h"
#include "RenderTarget.h"
#include "Texture.h"
#include "Shader.h"
#include "ImGui/GuiLayer.h"

struct GLFWwindow;
namespace Immortal
{
namespace OpenGL
{

class RenderContext : public SuperRenderContext
{
public:
    using Super = SuperRenderContext;

public:
    RenderContext(const SuperRenderContext::Description &desc);

    void Setup();

    virtual SuperGuiLayer *CreateGuiLayer() override;

    virtual void OnResize(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;

	virtual void SwapBuffers() override;

	virtual SuperTexture *CreateTexture(const std::string &filepath, const Texture::Description &description = {}) override;

	virtual SuperTexture *CreateTexture(uint32_t width, uint32_t height, const void *data, const Texture::Description &description) override;

	virtual SuperBuffer *CreateBuffer(const size_t size, uint32_t binding) override;

	virtual SuperBuffer *CreateBuffer(const size_t size, const void *data, Buffer::Type type) override;

	virtual SuperBuffer *CreateBuffer(const size_t size, Buffer::Type type) override;

	virtual SuperGraphicsPipeline *CreateGraphicsPipeline(Ref<SuperShader> shader) override;

	virtual SuperComputePipeline *CreateComputePipeline(Shader::Super *shader) override;

	virtual SuperRenderTarget*CreateRenderTarget(const RenderTarget::Description &description) override;

	virtual SuperShader *CreateShader(const std::string &filepath, Shader::Type type) override;

	virtual DescriptorBuffer *CreateImageDescriptor(uint32_t count) override;

	virtual void Draw(SuperGraphicsPipeline *pipeline) override;

	virtual void Begin(SuperRenderTarget *renderTarget) override;

	virtual void End() override;

	virtual void PushConstant(ComputePipeline *pipeline, uint32_t size, const void *data, uint32_t offset) override;

public:
	GLFWwindow *Handle()
	{
		return handle;
	}

protected:
    GLFWwindow *handle;
};

}
}
