#include "impch.h"
#include "RenderContext.h"

#include <array>
#include <GLFW/glfw3.h>

#include "Framework/Application.h"
#include "Renderer.h"
#include "Image.h"
#include "RenderFrame.h"

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
    /*VK_LAYER_LUNARG_API_DUMP,
    VK_LAYER_LUNARG_DEVICE_SIMULATION,
    VK_LAYER_LAYER_LUNARG_ASSISTANT_LAYER,
    VK_LAYER_KHRONOS_VALIDATION,
    VK_LAYER_LUNARG_MONITOR,
    VK_LAYER_LUNARG_SCREENSHOT*/
};

RenderContext::RenderContext(const RenderContext::Description &desc)
    : handle(desc.WindowHandle)
{
    if (!glfwVulkanSupported())
    {
        LOG::ERR("Vulkan Not Supported!");
        return;
    }

    instance = std::make_unique<Instance>(Application::Name(), InstanceExtensions, ValidationLayers);
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

    renderPass = std::make_unique<RenderPass>(device.get(), depthFormat);

    {
        surfaceExtent = VkExtent2D{ Application::Width(), Application::Height() };
        if (surface != VK_NULL_HANDLE)
        {
            VkSurfaceCapabilitiesKHR surfaceProperties;
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

        this->Set<VkFormat>(surfaceFormatPriorities[0].format);
        this->Set<VkPresentModeKHR>(presentModePriorities[0]);
        
        this->Prepare();
    }
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

    swapchain->Set(presentModePriorities);
    swapchain->Set(surfaceFormatPriorities);
    swapchain->Create();

    surfaceExtent = swapchain->Get<VkExtent2D>();
    VkExtent3D extent{ surfaceExtent.width, surfaceExtent.height, 1 };

    for (auto &handle : swapchain->Get<Swapchain::Images>())
    {
        Image image{ device.get(), handle, extent, swapchain->Get<VkFormat>(), swapchain->Get<VkImageUsageFlags>() };
        auto renderTarget = RenderTarget::Create(std::move(image));
        frames.emplace_back(std::make_unique<RenderFrame>(device.get(), std::move(renderTarget)));
    }

    this->threadCount = threadCount;

    semaphorePool.reset(new SemaphorePool{ &this->Get<Device>() });
    semaphores.acquiredImageReady = rcast<VkSemaphore>(semaphorePool->Request());
    semaphores.renderComplete     = rcast<VkSemaphore>(semaphorePool->Request());
    this->status = true;
}

void RenderContext::SwapBuffers()
{
   
}

}
}
