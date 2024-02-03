#include "Swapchain.h"
#include "Device.h"
#include "Metal/MTLTexture.hpp"
#include "Metal/Texture.h"
#include "Queue.h"
#include "Window.h"
#include "RenderTarget.h"

#include <Metal/Metal.hpp>
#include <Metal/Metal.h>
#include <QuartzCore/CAMetalLayer.hpp>
#include <QuartzCore/CAMetalLayer.h>
#include <QuartzCore/QuartzCore.h>
#include <QuartzCore/QuartzCore.hpp>

#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

namespace Immortal
{
namespace Metal
{

NSWindow *GetMetalWindow(Window *window)
{
    return glfwGetCocoaWindow((GLFWwindow *)window->GetBackendHandle());
}

Swapchain::Swapchain() :
    Handle<CA::MetalLayer>{},
    window{},
    drawable{},
    renderTarget{}
{

}

Swapchain::Swapchain(Device *device, Queue *queue, Window *window, Format format, uint32_t bufferCount, SwapchainMode swapchainMode) :
    Handle<CA::MetalLayer>{},
    window{ window },
    drawable{},
    renderTarget{}
{
    mode = swapchainMode;
    NSWindow *metalWindow = GetMetalWindow(window);

    handle = CA::MetalLayer::layer();
    handle->setDevice(*device);
    handle->setPixelFormat(format);

    CAMetalLayer *layer = (__bridge CAMetalLayer *)handle;
    metalWindow.contentView.layer      = layer;
    metalWindow.contentView.wantsLayer = YES;

    handle->setDrawableSize({ window->GetWidth(), window->GetHeight() });
    renderTarget = new RenderTarget{};
}

Swapchain::~Swapchain()
{
    if (drawable)
    {
        drawable->release();
        drawable = {};
    }
}

void Swapchain::PrepareNextFrame()
{
    if (drawable)
    {
        renderTarget->SetColorAttachment({});
        drawable->release();
        drawable = {};
    }
    
    auto width  = window->GetWidth();
    auto height = window->GetHeight();
    if (!renderTarget->InternalGetColorAttachments().empty() && (width != renderTarget->GetWidth() || height != renderTarget->GetHeight()))
    {
        handle->setDrawableSize({ width, height });
    }
    
    NSWindow *metalWindow = GetMetalWindow(window);
    drawable = handle->nextDrawable();
    if (drawable)
    {
        MTL::Texture *texture = drawable->texture();
        renderTarget->SetColorAttachment(texture);
        texture->autorelease();
        texture = {};
    }
}

void Swapchain::Resize(uint32_t width, uint32_t height)
{

}

SuperRenderTarget *Swapchain::GetCurrentRenderTarget()
{
    return renderTarget.Get();
}

void Swapchain::Present(Queue *queue)
{
    drawable->present();
}

}
}
