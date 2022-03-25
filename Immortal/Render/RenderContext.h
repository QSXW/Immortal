#pragma once

#include "Core.h"
#include "Framework/Window.h"
#include "ImGui/GuiLayer.h"
#include "Shader.h"

namespace Immortal
{

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

        Description(Window *handle, UINT32 width, UINT32 height, Swapchain::PresentMode mode = Swapchain::PresentMode::TripleBuffer) :
            WindowHandle{ handle },
            Width{ width },
            Height{ height },
            FrameCount{ ncast<int>(mode) }
        {

        }

        UINT32 Width{ 0 };

        UINT32 Height{ 0 };

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
    static std::unique_ptr<RenderContext> Create(const Description &desc);

public:
    RenderContext()
    {

    }

    virtual void SwapBuffers()
    {

    }

    virtual bool HasSwapchain()
    {
        return false;
    }

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

    std::shared_ptr<Shader> CreateShader(const std::string &filename, Shader::Type type);

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
    std::string graphicsRenderer{};
    std::string driverVersion{};
    std::string vendor{};
};

using SuperRenderContext = RenderContext;

}
