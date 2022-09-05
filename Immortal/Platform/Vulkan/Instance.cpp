#include "Instance.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Device.h"
#include "PhysicalDevice.h"
#include "RenderContext.h"
#include "Framework/Window.h"

namespace Immortal
{
namespace Vulkan
{

static VKAPI_ATTR VkBool32 VKAPI_CALL DebugUtilsMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                                    VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                                    const VkDebugUtilsMessengerCallbackDataEXT *callbackData,
                                                                    void *userData)
{
    switch (messageSeverity)
    {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        LOG::INFO("{} - {}: {}", callbackData->messageIdNumber, callbackData->pMessageIdName, callbackData->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        LOG::WARN("{} - {}: {}", callbackData->messageIdNumber, callbackData->pMessageIdName, callbackData->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
    default:
        LOG::ERR("{} - {}: {}", callbackData->messageIdNumber, callbackData->pMessageIdName, callbackData->pMessage);
        break;
    }

    return VK_FALSE;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT /*type*/,
                                                    uint64_t /*object*/,  size_t /*location*/, int32_t /*message_code*/,
                                                    const char *layerPrefix, const char *message,void * /*userData*/)
{
    switch (flags)
    {
    case VK_DEBUG_REPORT_ERROR_BIT_EXT:
        LOG::ERR("{}: {}", layerPrefix, message);
        break;
    case VK_DEBUG_REPORT_WARNING_BIT_EXT:
    case VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT:
        LOG::WARN("{}: {}", layerPrefix, message);
        break;
    default:
        LOG::INFO("{}: {}", layerPrefix, message);
        break;
    }

    return VK_FALSE;
}

static bool ValidateLayers(const std::vector<const char *> &required, const std::vector<VkLayerProperties> &available)
{
    for (auto &layer : required)
    {
        bool found = false;
        for (auto &availableLayer : available)
        {
            if (Equals(layer, availableLayer.layerName))
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            LOG::ERR("Validation Layer {} not found", layer);
            return false;
        }
    }

    return true;
}

static inline void CheckDynamicLoadedLibarary()
{
    if (RenderContext::Status == VK_NOT_READY)
    {
        if ((RenderContext::Status = volkInitialize()) != VK_SUCCESS)
        {
            LOG::FATAL("Unable to initialize Vulkan Library. Please confirm your device support "
                        "and get Vulkan Installed");
            Check(RenderContext::Status);
        }
        LOG::INFO("Load Vulkan library with success...");
    }
}

static inline void VkEnableExtension(const char *message)
{
    LOG::INFO("{0} is available. Enabling it!", message);
}

Instance::Instance(const char                                   *applicationName,
                    const std::unordered_map<const char *, bool> &requiredExtension,
                    const std::vector<const char *>              &requiredValidationLayers,
                    bool                                          headless,
                    uint32_t                                      apiVersion)
{
    CheckDynamicLoadedLibarary();

    uint32_t extensionCount;
    Check(vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr));
    std::vector<VkExtensionProperties> availableExtension{ extensionCount };
    Check(vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtension.data()));

    uint64_t extensionsFlags = 0;
    static std::vector<const char*> utilsExtensions = {
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
        VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME,
        VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
    };

    for (size_t i = 0; i < utilsExtensions.size(); i++)
    {
        auto &extension = utilsExtensions[i];
        for (size_t j = 0; j < availableExtension.size(); j++)
        {
            if (Equals(availableExtension[j].extensionName, extension))
            {
                extensionsFlags |= BIT(i);
                VkEnableExtension(extension);
                enabledExtensions.push_back(extension);
            }
        }
    }

    if (headless && !(extensionsFlags & BIT(1)))
    {
        LOG::WARN("{0} is not available. Disabling swapchain creation", VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME);
    }
    else
    {
        enabledExtensions.emplace_back(VK_KHR_SURFACE_EXTENSION_NAME);
    }

    if (!(extensionsFlags & BIT(0)) /* not debug utils */)
    {
        enabledExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    }

    for (auto &requird : requiredExtension)
    {
        auto extension  = requird.first;
        auto isOptional = requird.second;

        if (std::find_if(availableExtension.begin(), availableExtension.end(), [&extension](VkExtensionProperties &availableExtension)
            {
                return Equals(availableExtension.extensionName, extension);
            }) == availableExtension.end())
        {
            if (isOptional)
            {
                LOG::WARN("Optional instance extension {0} not available, some features may be disabled", extension);
            }
            else
            {
                LOG::ERR("Required instance extension {0} not available, cannot run", extension);
                LOG::ERR("Required instance extensions are missing.");
            }
        }
        else
        {
            enabledExtensions.emplace_back(extension);
        }
    }

    uint32_t layerCount;
    Check(vkEnumerateInstanceLayerProperties(&layerCount, nullptr));

    std::vector<VkLayerProperties> supportedValidationLayers(layerCount);
    Check(vkEnumerateInstanceLayerProperties(&layerCount, supportedValidationLayers.data()));

    if (ValidateLayers(requiredValidationLayers, supportedValidationLayers))
    {
        LOG::INFO("Enabled Validation Layers: ");
        for (const auto &layer : requiredValidationLayers)
        {
            LOG::DEBUG("  \t{0}", layer);
        }
    }
    else
    {
        LOG::ERR("Required validation layers are missing.");
    }

    VkApplicationInfo appInfo{};
    appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pNext              = nullptr;
    appInfo.pApplicationName   = applicationName;
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName        = applicationName;
    appInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion         = apiVersion;

    VkInstanceCreateInfo instanceInfo = {
        .sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext                   = nullptr,
#ifdef VK_KHR_portability_enumeration
        .flags                   = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR,
#endif
        .pApplicationInfo        = &appInfo,
        .enabledLayerCount       = U32(requiredValidationLayers.size()),
        .ppEnabledLayerNames     = requiredValidationLayers.data(),
	    .enabledExtensionCount   = U32(enabledExtensions.size()),
	    .ppEnabledExtensionNames = enabledExtensions.data(),
    };

#ifdef _DEBUG
    VkDebugUtilsMessengerCreateInfoEXT debugUtilsCreateInfo = { VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };
    VkDebugReportCallbackCreateInfoEXT debugReportCreateInfo = { VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT };

    if (extensionsFlags & BIT(0))
    {
        debugUtilsCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
        debugUtilsCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debugUtilsCreateInfo.pfnUserCallback =DebugUtilsMessengerCallback;

        instanceInfo.pNext = &debugUtilsCreateInfo;
    }
    else
    {
        debugReportCreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
        debugReportCreateInfo.pfnCallback =DebugCallback;

        instanceInfo.pNext = &debugReportCallback;
    }
#endif

    Check(vkCreateInstance(&instanceInfo, nullptr, &handle));
    volkLoadInstance(handle);

#ifdef _DEBUG
    if (extensionsFlags & BIT(0))
    {
        SLASSERT(vkCreateDebugUtilsMessengerEXT != nullptr && "");
        Check(vkCreateDebugUtilsMessengerEXT(handle, &debugUtilsCreateInfo, nullptr, &debugUtilsMessengers));
    }
    else
    {
        SLASSERT(vkCreateDebugReportCallbackEXT != nullptr && "");
        Check(vkCreateDebugReportCallbackEXT(handle, &debugReportCreateInfo, nullptr, &debugReportCallback));
    }
#endif
    QueryPhysicalDevice();
}

Instance::Instance(VkInstance instance) :
    handle(instance)
{
    if (handle != VK_NULL_HANDLE)
    {
        throw RuntimeException("Instance is not valid!");
    }
    QueryPhysicalDevice();
}

Instance::~Instance()
{
#ifdef _DEBUG
    if (debugUtilsMessengers)
    {
        vkDestroyDebugUtilsMessengerEXT(handle, debugUtilsMessengers, nullptr);
    }
    if (debugReportCallback)
    {
        vkDestroyDebugReportCallbackEXT(handle, debugReportCallback, nullptr);
    }
#endif
    if (handle)
    {
        IfNotNullThen<VkInstance>(vkDestroyInstance, handle, nullptr);
    }
}

void Instance::QueryPhysicalDevice()
{
    uint32_t count{ 0 };
    Check(EnumeratePhysicalDevices(&count, nullptr));

    ThrowIf(!count, "Couldn't find a physical device that supports Vulkan.");

    std::vector<VkPhysicalDevice> physicalDevices{ count };
    Check(EnumeratePhysicalDevices(&count, physicalDevices.data()));

    for (auto &pd : physicalDevices)
    {
        this->physicalDevices.emplace_back(std::make_unique<PhysicalDevice>(this, pd));
    }
}

PhysicalDevice &Instance::SuitablePhysicalDevice()
{
    ThrowIf(physicalDevices.empty(), "There is no GPU on this device.");

    for (auto &physicalDevice : physicalDevices)
    {
        if (physicalDevice->Properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        {
            return *physicalDevice;
        }
    }

    if (physicalDevices[0]->Properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU)
    {
        throw RuntimeException("No physical device available on this host!");
    }

    LOG::WARN("Couldn't find a discrete physical device. Picking the default.");
    return *physicalDevices.at(0);
}

VkResult Instance::CreateSurface(Window *window, VkSurfaceKHR *pSurface, const VkAllocationCallbacks *pAllocator) const
{
    if (!window || window->GetType() == Window::Type::Headless)
    {
        LOG::WARN("Failed to create surface without valid window. Enable the headless Vulkan rendering.");
        *pSurface = VK_NULL_HANDLE;
        return VK_SUCCESS;
    }

    switch (window->GetType())
    {
    case  Window::Type::GLFW:
        return glfwCreateWindowSurface(handle, static_cast<GLFWwindow *>(window->GetNativeWindow()), pAllocator, pSurface);

    default:
#ifdef _WIN32
        VkWin32SurfaceCreateInfoKHR createInfo{};
        createInfo.sType     = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        createInfo.hinstance = GetModuleHandle(NULL);
        createInfo.hwnd      = static_cast<HWND>(window->Primitive());
        return vkCreateWin32SurfaceKHR(handle, &createInfo, pAllocator, nullptr);
#else
        LOG::WARN("Native surface creation is not supported on Linux and Mac yet!");
        return VK_ERROR_INITIALIZATION_FAILED;
#endif
    }
}

}
}
