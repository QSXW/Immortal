#include "impch.h"
#include "RenderContext.h"

#include <array>

#include "Common.h"
#include <GLFW/glfw3.h>

#include "Framework/Application.h"
#include "Renderer.h"
#include "Image.h"
#include "RenderFrame.h"
#include "Framebuffer.h"

namespace Immortal
{

namespace Vulkan
{
VkResult RenderContext::Status = VK_NOT_READY;

std::unordered_map<const char *, bool> RenderContext::InstanceExtensions{
    { IMMORTAL_PLATFORM_SURFACE, false }
};

std::unordered_map<const char *, bool> RenderContext::DeviceExtensions{
    { VK_KHR_SWAPCHAIN_EXTENSION_NAME, false }
};

static std::vector<const char *> ValidationLayers = {
    // "VK_LAYER_RENDERDOC_Capture",
    // VK_LAYER_LUNARG_API_DUMP,
    // VK_LAYER_LUNARG_DEVICE_SIMULATION,
    // VK_LAYER_LAYER_LUNARG_ASSISTANT_LAYER,
    VK_LAYER_KHRONOS_VALIDATION,
    "VK_LAYER_KHRONOS_synchronization2",
    // VK_LAYER_LUNARG_MONITOR,
    // VK_LAYER_LUNARG_SCREENSHOT
};

RenderContext::RenderContext(const RenderContext::Description &desc)
    : handle{ desc.WindowHandle->GetNativeWindow() }
{
    instance = std::make_unique<Instance>(Application::Name(), InstanceExtensions, ValidationLayers);
    if (!instance)
    {
        LOG::ERR("Vulkan Not Supported!");
        return;
    }
    CreateSurface();

    auto &physicalDevice = instance->SuitablePhysicalDevice();
    physicalDevice.HighPriorityGraphicsQueue            = true;
    physicalDevice.RequestedFeatures.samplerAnisotropy  = VK_TRUE;
    physicalDevice.RequestedFeatures.robustBufferAccess = VK_TRUE;

    depthFormat = SuitableDepthFormat(physicalDevice.Handle());

    if (physicalDevice.Features.textureCompressionASTC_LDR)
    {
        physicalDevice.RequestedFeatures.textureCompressionASTC_LDR = VK_TRUE;
    }

    if (instance->IsEnabled(VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME))
    {
        AddDeviceExtension(VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME);
    }

    device = std::make_unique<Device>(physicalDevice, surface, DeviceExtensions);
    queue  = device->SuitableGraphicsQueuePtr();

    surfaceExtent = VkExtent2D{ Application::Width(), Application::Height() };
    if (surface != VK_NULL_HANDLE)
    {
        VkSurfaceCapabilitiesKHR surfaceProperties{};
        Check(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice.Handle(), surface, &surfaceProperties));

        if (surfaceProperties.currentExtent.width == 0xFFFFFFFF)
        {
            swapchain = std::make_unique<Swapchain>(*device, surface, surfaceExtent);
        }
        else
        {
            swapchain = std::make_unique<Swapchain>(*device, surface);
        }
    }
    Prepare();

    Super::UpdateMeta(physicalDevice.Properties.deviceName, nullptr, nullptr);
}

RenderContext::~RenderContext()
{
    LOG::INFO("Application Closed => deconstruct RenderContex.");
    vkDestroySurfaceKHR(instance->Handle(), surface, nullptr);
    instance.release();
}

void RenderContext::CreateSurface()
{
    if (instance->Handle() == VK_NULL_HANDLE && !handle)
    {
        surface = VK_NULL_HANDLE;
        return;
    }

    Check(glfwCreateWindowSurface(instance->Handle(), static_cast<GLFWwindow *>(handle), nullptr, &surface));
}

void RenderContext::Prepare(size_t threadCount)
{
    device->Wait();

    swapchain->Create();

    renderPass  = std::make_unique<RenderPass>(device.get(), swapchain->Get<VkFormat>(), depthFormat);

    surfaceExtent = swapchain->Get<VkExtent2D>();
    VkExtent3D extent{ surfaceExtent.width, surfaceExtent.height, 1 };
    
    std::vector<ImageView> views;

    for (auto &handle : swapchain->Get<Swapchain::Images>())
    {
        Image image{ 
            device.get(),
            handle,
            extent,
            swapchain->Get<VkFormat>(),
            swapchain->Get<VkImageUsageFlags>()
            };
        auto renderTarget = RenderTarget::Create(std::move(image));
        present.framebuffers.emplace_back(std::make_unique<Framebuffer>(device.get(), renderPass.get(), renderTarget->Views(), surfaceExtent));
        frames.emplace_back(std::make_unique<RenderFrame>(device.get(), std::move(renderTarget)));
    }
    this->threadCount = threadCount;

    for (auto &buf : present.commandBuffers)
    {
        buf.reset(device->Request(Level::Primary));
    }

    this->status = true;
}

Swapchain *RenderContext::UpdateSurface()
{
    if (!swapchain)
    {
        goto end;
    }

    VkSurfaceCapabilitiesKHR properties;
    Check(device->GetSurfaceCapabilities(swapchain->Get<Surface>(), &properties));
    
    if (properties.currentExtent.width == 0xFFFFFFFF || (properties.currentExtent.height <= 0 && properties.currentExtent.width <= 0))
    {
        goto end;
    }
    if (properties.currentExtent.width != surfaceExtent.width || properties.currentExtent.height != surfaceExtent.height)
    {
        device->Wait();
        surfaceExtent = properties.currentExtent;

        // LOG::INFO("Destory swapchain => {0}", (void *)swapchain->Handle());
        UpdateSwapchain(surfaceExtent, regisry.preTransform);
        // LOG::INFO("Create  swapchain => {0}", (void *)swapchain->Handle());
    }

end:
    return swapchain.get();
}

void RenderContext::UpdateSwapchain(const VkExtent2D &extent, const VkSurfaceTransformFlagBitsKHR transform)
{
    present.framebuffers.clear();

    if (transform == VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR || transform == VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR)
    {
        swapchain = std::make_unique<Swapchain>(*swapchain, VkExtent2D{ extent.height, extent.width }, transform);
    }
    else
    {
        swapchain = std::make_unique<Swapchain>(*swapchain, extent, transform);;
    }
    regisry.preTransform = transform;

    auto frame = frames.begin();
    for (auto &handle : swapchain->Get<Swapchain::Images>())
    {
        Image image{
            device.get(),
            handle,
            VkExtent3D{ extent.width, extent.height, 1 },
            swapchain->Get<VkFormat>(),
            swapchain->Get<VkImageUsageFlags>()
        };

        // LOG::INFO("Create Render Target{0}", (void*)image.Handle());
        auto renderTarget = RenderTarget::Create(std::move(image));
        
        present.framebuffers.emplace_back(std::make_unique<Framebuffer>(device.get(), renderPass.get(), renderTarget->Views(), surfaceExtent));

        if (frame != frames.end())
        {
            (*frame)->Set(renderTarget);
        }
        else
        {
            frames.emplace_back(std::make_unique<RenderFrame>(device.get(), std::move(renderTarget), threadCount));
        }
        
        frame++;
    }
}

void RenderContext::SwapBuffers()
{
    
}

}
}
