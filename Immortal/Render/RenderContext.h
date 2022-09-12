#pragma once

#include "Core.h"
#include "Framework/Window.h"
#include "Shader.h"
#include "Pipeline.h"
#include "RenderTarget.h"

namespace Immortal
{

class GuiLayer;
class Swapchain
{
public:
enum class PresentMode
{
    DoubleBuffer = 2,
    TripleBuffer = 3
};
};

class Shader;
class IMMORTAL_API RenderContext
{
public:
    struct Description
    {
        Description() { }

        Description(Window *handle, uint32_t width, uint32_t height, Swapchain::PresentMode mode = Swapchain::PresentMode::TripleBuffer) :
            WindowHandle{ handle },
            Width{ width },
            Height{ height },
            FrameCount{ ncast<int>(mode) }
        {

        }

        uint32_t Width{ 0 };

        uint32_t Height{ 0 };

        int FrameCount{ 3 };

        Window *WindowHandle{ nullptr };

        Format format{ Format::RGBA8 };

        const char *ApplicationName{ "" };

        void Set(Swapchain::PresentMode mode)
        {
            FrameCount = ncast<int>(mode);
        }
    };

public:
    static RenderContext *Create(const Description &desc);

public:
    RenderContext()
    {

    }

    virtual ~RenderContext() = default;

    virtual GuiLayer *CreateGuiLayer()
    {
        return nullptr;
    }

    virtual const char *GraphicsRenderer() const
    {
        return graphicsRenderer.c_str();
    }

    virtual const char *DriverVersion() const
    {
        return driverVersion.c_str();
    }

    virtual const char *VendorID() const
    {
        return vendor.c_str();
    }

	virtual void OnResize(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{
	
	}

    virtual void SwapBuffers()
	{
	
	}

	virtual void PrepareFrame()
	{
	
	}

	virtual Shader *CreateShader(const std::string &filepath, Shader::Type type)
	{
		return nullptr;
	}

	virtual GraphicsPipeline *CreateGraphicsPipeline(Ref<Shader> shader)
	{
		return nullptr;
	}

	virtual ComputePipeline *CreateComputePipeline(Shader *shader)
	{
		return nullptr;
	}

	virtual Texture *CreateTexture(const std::string &filepath, const Texture::Description &description = {})
	{
		return nullptr;
	}

	virtual Texture *CreateTexture(uint32_t width, uint32_t height, const void *data, const Texture::Description &description = {})
	{
		return nullptr;
	}

	virtual TextureCube *CreateTextureCube(uint32_t width, uint32_t height, const Texture::Description &description = {})
	{
		return nullptr;
	}

	virtual Buffer *CreateBuffer(const size_t size, const void *data, Buffer::Type type)
	{
		return nullptr;
	}

	virtual Buffer *CreateBuffer(const size_t size, Buffer::Type type)
	{
		return nullptr;
	}

	virtual Buffer *CreateBuffer(const size_t size, uint32_t binding)
	{
		return nullptr;
	}

	virtual RenderTarget *CreateRenderTarget(const RenderTarget::Description &description)
	{
		return nullptr;
	}

	virtual void PushConstant(GraphicsPipeline *pipeline, Shader::Stage stage, uint32_t size, const void *data, uint32_t offset)
	{
	}

	virtual void PushConstant(ComputePipeline *pipeline, uint32_t size, const void *data, uint32_t offset)
	{
	}

	virtual void Draw(GraphicsPipeline *pipeline)
	{
	}

	virtual void Dispatch(ComputePipeline *superPipeline, uint32_t nGroupX, uint32_t nGroupY, uint32_t nGroupZ)
	{
	}

	virtual void Begin(RenderTarget *renderTarget)
	{
	}

	virtual void End()
	{
	}

	virtual DescriptorBuffer *CreateImageDescriptor(uint32_t count)
	{
		return nullptr;
	}

	virtual DescriptorBuffer *CreateBufferDescriptor(uint32_t count)
	{
		return nullptr;
	}

public:
    RenderContext(const RenderContext &) = delete;
    RenderContext(RenderContext &&) = delete;
    RenderContext &operator=(const RenderContext &) = delete;
    RenderContext &operator=(RenderContext &&) = delete;

    void UpdateMeta(const char *physicalDevice, const char *version, const char *vendor)
    {
        if (physicalDevice)
        {
            this->graphicsRenderer = physicalDevice;
        }
        if (version)
        {
            this->driverVersion = version;
        }
        if (vendor)
        {
            this->vendor = vendor;
        }
    }

protected:
    std::string graphicsRenderer;
    std::string driverVersion;
    std::string vendor;
};

using SuperRenderContext = RenderContext;

namespace Interface
{
	using RenderContext = SuperRenderContext;
}

}
