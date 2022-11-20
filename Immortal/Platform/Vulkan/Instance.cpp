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


static void EnumerateValidateLayers(const std::vector<Instance::Extension> &required, std::vector<Instance::Extension> &enabledValidationLayers)
{
#ifdef _DEBUG
    uint32_t layerCount = 0;
	Check(EnumerateInstanceLayerProperties(&layerCount, nullptr));

	std::vector<VkLayerProperties> availableLayers{layerCount};
	Check(EnumerateInstanceLayerProperties(&layerCount, availableLayers.data()));

    for (auto &layer : required)
    {
		int i;
		for (i = 0; i < availableLayers.size(); i++)
        {
			if (Equals(layer, availableLayers[i].layerName))
            {
				enabledValidationLayers.emplace_back(layer);
				break;
            }
        }
        if (i == availableLayers.size())
        {
		    LOG::ERR("Required Validation Layer is not supported - {}", layer);
        }
    }
#endif
}

static inline void LoadVulkanModule()
{
    if (!vkGetInstanceProcAddr)
    {
        if ((RenderContext::Status = volkInitialize()) != VK_SUCCESS)
        {
            throw RuntimeException("Unable to initialize Vulkan Library. Please confirm your device support "
                        "and get Vulkan Installed");
            Check(RenderContext::Status);
        }
        LOG::INFO("Load Vulkan library with success...");
    }
}

enum class InternalExtension : uint64_t
{
    None = 0,
    DebugUtils = BIT(0),
    HeadlessSurface = BIT(1),
    GetPhysicalDeviceProperties2 = BIT(2)
};

SL_ENABLE_BITWISE_OPERATOR(InternalExtension)

Instance::Instance() :
    handle{}
#ifdef _DEBUG
    , debugUtilsMessengers{},
    debugReportCallback{}
#endif
{
    LoadVulkanModule();
}

Instance::Instance(const char                                   *applicationName,
                    const std::unordered_map<const char *, bool> &requiredExtension,
                    const std::vector<const char *>              &requiredValidationLayers,
                    bool                                          headless,
                    uint32_t                                      apiVersion) :
    Instance{}
{
    uint32_t extensionCount;
    Check(EnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr));
    std::vector<VkExtensionProperties> availableExtension{ extensionCount };
    Check(EnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtension.data()));

    InternalExtension extensionsFlags = InternalExtension::None;
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
                extensionsFlags |= (InternalExtension)BIT(i);
                LOG::INFO("Extension Enabled: {}", extension);
                enabledExtensions.push_back(extension);
            }
        }
    }

    if (headless && !(extensionsFlags & InternalExtension::HeadlessSurface))
    {
        LOG::WARN("{} is not available. Disabling swapchain creation", VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME);
    }
    else
    {
        enabledExtensions.emplace_back(VK_KHR_SURFACE_EXTENSION_NAME);
    }

    if (!(extensionsFlags & InternalExtension::DebugUtils) /* not debug utils */)
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
                LOG::WARN("Optional instance extension {} not available, some features may be disabled", extension);
            }
            else
            {
                LOG::ERR("Required instance extension {} not available, cannot run", extension);
                LOG::ERR("Required instance extensions are missing.");
            }
        }
        else
        {
            enabledExtensions.emplace_back(extension);
        }
    }

    std::vector<const char *> validationLayers;
    EnumerateValidateLayers(requiredValidationLayers, validationLayers);

    VkApplicationInfo appInfo = {
        .sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext              = nullptr,
        .pApplicationName   = applicationName,
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName        = applicationName,
        .engineVersion      = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion         = apiVersion,
    };

    VkInstanceCreateInfo instanceInfo = {
        .sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext                   = nullptr,
#ifdef __APPLE__
        .flags                   = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR,
#endif
        .pApplicationInfo        = &appInfo,
        .enabledLayerCount       = U32(validationLayers.size()),
        .ppEnabledLayerNames     = validationLayers.data(),
	    .enabledExtensionCount   = U32(enabledExtensions.size()),
	    .ppEnabledExtensionNames = enabledExtensions.data(),
    };

#ifdef _DEBUG
    VkDebugUtilsMessengerCreateInfoEXT debugUtilsCreateInfo = { VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };
    VkDebugReportCallbackCreateInfoEXT debugReportCreateInfo = { VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT };

    if (!!(extensionsFlags & InternalExtension::DebugUtils))
    {
        debugUtilsCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
        debugUtilsCreateInfo.messageType     = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debugUtilsCreateInfo.pfnUserCallback = DebugUtilsMessengerCallback;

        instanceInfo.pNext = &debugUtilsCreateInfo;
    }
    else
    {
        debugReportCreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
        debugReportCreateInfo.pfnCallback = DebugCallback;

        instanceInfo.pNext = &debugReportCallback;
    }
#endif

    Check(CreateInstance(&instanceInfo, &handle));
    volkLoadInstance(handle);

#ifdef _DEBUG
    if (!!(extensionsFlags & InternalExtension::DebugUtils))
    {
        SLASSERT(vkCreateDebugUtilsMessengerEXT != nullptr);
        Check(CreateDebugUtilsMessengerEXT(&debugUtilsCreateInfo, &debugUtilsMessengers));
    }
    else
    {
        SLASSERT(vkCreateDebugReportCallbackEXT != nullptr);
        Check(CreateDebugReportCallbackEXT(&debugReportCreateInfo, &debugReportCallback));
    }
#endif

    EnumeratePhysicalDevice();
}

Instance::Instance(VkInstance instance) :
    handle(instance)
{
    if (handle != VK_NULL_HANDLE)
    {
        throw RuntimeException("Instance is not valid!");
    }
    EnumeratePhysicalDevice();
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

Instance::Instance(Instance &&other) :
    Instance{}
{
    other.Swap(*this);

    for (auto &physicalDevice : physicalDevices)
    {
        physicalDevice.Invalidate(this);
    }
}

Instance &Instance::operator =(Instance &&other)
{
    Instance(std::move(other)).Swap(*this);

    for (auto &physicalDevice : physicalDevices)
    {
        physicalDevice.Invalidate(this);
    }

    return *this;
}

void Instance::Swap(Instance &other)
{
    std::swap(handle,            other.handle           );
    std::swap(enabledExtensions, other.enabledExtensions);
    std::swap(physicalDevices,   other.physicalDevices  );

#if defined (_DEBUG) || defined (VKB_VALIDATION_LAYERS)
    std::swap(debugUtilsMessengers, other.debugUtilsMessengers);
    std::swap(debugReportCallback,  other.debugReportCallback );
#endif
}

void Instance::EnumeratePhysicalDevice()
{
    uint32_t count = 0;
    Check(EnumeratePhysicalDevices(&count, nullptr));

    ThrowIf(!count, "Couldn't find a physical device that supports Vulkan.");

    std::vector<VkPhysicalDevice> physicalDevices{ count };
    Check(EnumeratePhysicalDevices(&count, physicalDevices.data()));

    for (auto &pd : physicalDevices)
    {
        this->physicalDevices.emplace_back(this, pd);
    }
}

PhysicalDevice *Instance::GetSuitablePhysicalDevice(int deviceId)
{
    ThrowIf(physicalDevices.empty(), "There is no GPU on this device.");

    if (deviceId != AUTO_DEVICE_ID)
    {
        if (deviceId >= physicalDevices.size())
        {
            LOG::ERR("Failed to query physical device with `deviceId:{}`", deviceId);
            throw RuntimeException("Invalid Device Id");
        }

        return &physicalDevices[deviceId];
    }

    for (auto &physicalDevice : physicalDevices)
    {
        if (physicalDevice.Properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        {
            return &physicalDevice;
        }
    }

    if (physicalDevices[0].Properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU)
    {
        throw RuntimeException("No physical device available on this host!");
    }

    LOG::WARN("Couldn't find a discrete physical device. Picking the default.");
    return &physicalDevices[0];
}

bool Instance::IsEnabled(Extension extension) const
{
    return std::find_if(enabledExtensions.begin(), enabledExtensions.end(),
        [extension](Extension enabledExtension)
        {
            return Equals(extension, enabledExtension);
        }) != enabledExtensions.end();
}

VkResult Instance::CreateSurface(VkInstance instance, Anonymous window, VkSurfaceKHR *pSurface, const VkAllocationCallbacks *pAllocator)
{
#ifdef _WIN32
        VkWin32SurfaceCreateInfoKHR createInfo{};
        createInfo.sType     = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        createInfo.hinstance = GetModuleHandle(NULL);
        createInfo.hwnd      = (HWND)window;
        return vkCreateWin32SurfaceKHR(instance, &createInfo, pAllocator, pSurface);
#else
        LOG::WARN("Native surface creation is not supported on Linux and Mac yet!");
        return VK_ERROR_INITIALIZATION_FAILED;
#endif
}

VkResult Instance::CreateSurface(Window *window, VkSurfaceKHR *pSurface, const VkAllocationCallbacks *pAllocator)
{
    if (!window || window->GetType() == Window::Type::Headless)
    {
        LOG::INFO("Failed to create surface without valid window. Enable the headless Vulkan rendering.");
        *pSurface = VK_NULL_HANDLE;
        return VK_SUCCESS;
    }

    switch (window->GetType())
    {
    case  Window::Type::GLFW:
        return glfwCreateWindowSurface(handle, (GLFWwindow *)(window->Primitive()), pAllocator, pSurface);

    case Window::Type::Win32:
    default:
        return CreateSurface(*this, window->Primitive(), pSurface);
    }

}

}
}
