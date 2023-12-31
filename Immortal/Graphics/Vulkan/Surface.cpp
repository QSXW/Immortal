#include "Surface.h"
#include "Instance.h"
#include "Shared/DLLLoader.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#ifdef IMMORTAL_VK_USE_PLATFORM_XCB_KHR
#define GLFW_EXPOSE_NATIVE_X11
#endif

#ifdef IMMORTAL_VK_USE_PLATFORM_XLIB_KHR
#ifndef GLFW_EXPOSE_NATIVE_X11
#define GLFW_EXPOSE_NATIVE_X11
#endif
#endif

#ifdef VK_USE_PLATFORM_WAYLAND_KHR
#define GLFW_EXPOSE_NATIVE_WAYLAND
#endif

#if defined(GLFW_EXPOSE_NATIVE_WAYLAND) || defined(GLFW_EXPOSE_NATIVE_X11)
#include <GLFW/glfw3native.h>
#endif

#ifdef IMMORTAL_VK_USE_PLATFORM_XCB_KHR
using VkXcbSurfaceCreateFlagsKHR = VkFlags;
using xcb_window_t = XID;
struct xcb_connection_t;
typedef struct VkXcbSurfaceCreateInfoKHR
{
    VkStructureType             sType;
    const void                 *pNext;
    VkXcbSurfaceCreateFlagsKHR  flags;
    xcb_connection_t           *connection;
    xcb_window_t                window;
} VkXcbSurfaceCreateInfoKHR;
#endif

#ifdef IMMORTAL_VK_USE_PLATFORM_XLIB_KHR
using VkXlibSurfaceCreateFlagsKHR = VkFlags;
struct VkXlibSurfaceCreateInfoKHR
{
    VkStructureType              sType;
    const void                  *pNext;
    VkXlibSurfaceCreateFlagsKHR  flags;
    Display                     *dpy;
    Window                       window;
};
#endif

namespace Immortal
{

namespace Vulkan
{

Surface::Surface() :
    Handle{}
{

}

Surface::~Surface()
{
    assert(handle == VK_NULL_HANDLE && "Vulkan surface is not released!");
}

VkResult Surface::Create(Instance *instance, Window *window, const VkAllocationCallbacks *pAllocator)
{
    if (!window || window->GetType() == Window::Type::Headless)
    {
        LOG::INFO("Failed to create surface without valid window. Enable the headless Vulkan rendering.");
        return VK_SUCCESS;
    }

    Window::Type type = window->GetType();
	return CreateObject(*instance, type, window->GetBackendHandle(), &handle, nullptr);
}

void Surface::Release(Instance *instance)
{
    if (handle != VK_NULL_HANDLE)
    {
        instance->DestroySurfaceKHR(*this, nullptr);
        handle = VK_NULL_HANDLE;
    }
}

VkResult Surface::CreateObject(VkInstance instance, Window::Type type, Anonymous window, VkSurfaceKHR *pSurface, const VkAllocationCallbacks *pAllocator)
{
    switch (type)
    {
    case Window::Type::GLFW:
        return glfwCreateWindowSurface(instance, (GLFWwindow *)window, pAllocator, pSurface);

#ifdef VK_USE_PLATFORM_WIN32_KHR
        case Window::Type::Win32:
        {
            VkWin32SurfaceCreateInfoKHR createInfo{};
            createInfo.sType     = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
            createInfo.hinstance = GetModuleHandle(NULL);
            createInfo.hwnd      = (HWND)window;
            return vkCreateWin32SurfaceKHR(instance, &createInfo, pAllocator, pSurface);
        }
#endif
#ifdef IMMORTAL_VK_USE_PLATFORM_XCB_KHR
        case Window::Type::XCB:
        {
            using PFN_XGetXCBConnection = xcb_connection_t *(*)(Display *);
            PFN_XGetXCBConnection XGetXCBConnection = nullptr;

            static const std::vector<std::string> xcbLibraries = {
                "libX11-xcb.so.1",
                "libX11-xcb-1.so",
                "libX11-xcb.so",
            };

            for (auto &so : xcbLibraries)
            {
                DLLLoader loader(so);
                if (loader)
                {
                    XGetXCBConnection = loader.GetFunc<PFN_XGetXCBConnection>("XGetXCBConnection");
                    break;
                }
            }

            if (!XGetXCBConnection)
            {
                LOG::ERR("Surface: Unable to load any of libX11-xcb library to load XGetXCBConnection");
                return VK_ERROR_FEATURE_NOT_PRESENT;
            }

            using PFN_vkCreateXcbSurfaceKHR = VkResult (APIENTRY *)(VkInstance, const VkXcbSurfaceCreateInfoKHR *, const VkAllocationCallbacks*, VkSurfaceKHR *);
            auto vkCreateXcbSurfaceKHR = (PFN_vkCreateXcbSurfaceKHR)vkGetInstanceProcAddr(instance, "vkCreateXcbSurfaceKHR");
            if (!vkCreateXcbSurfaceKHR)
            {
                LOG::ERR("Vulkan: instance missing VK_KHR_xcb_surface extension");
                return VK_ERROR_EXTENSION_NOT_PRESENT;
            }

            VkXcbSurfaceCreateInfoKHR createInfo {
                .sType      = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR,
                .pNext      = nullptr,
                .flags      = 0,
                .connection = XGetXCBConnection(glfwGetX11Display()),
                .window     = glfwGetX11Window((GLFWwindow *)window),
            };

            return vkCreateXcbSurfaceKHR(instance, &createInfo, pAllocator, pSurface);
        }
#endif
#ifdef IMMORTAL_VK_USE_PLATFORM_XLIB_KHR
        case Window::Type::X11:
        {
            using PFN_vkCreateXlibSurfaceKHR = VkResult (APIENTRY *)(VkInstance, const VkXlibSurfaceCreateInfoKHR *, const VkAllocationCallbacks*, VkSurfaceKHR *);
            auto vkCreateXlibSurfaceKHR = (PFN_vkCreateXlibSurfaceKHR)vkGetInstanceProcAddr(instance, "vkCreateXlibSurfaceKHR");
            if (!vkCreateXlibSurfaceKHR)
            {
                LOG::ERR("Vulkan: instance missing VK_KHR_xlib_surface extension");
                return VK_ERROR_EXTENSION_NOT_PRESENT;
            }

            VkXlibSurfaceCreateInfoKHR createInfo {
                .sType   = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
                .pNext   = nullptr,
                .flags   = 0,
                .dpy     = glfwGetX11Display(),
                .window  = glfwGetX11Window((GLFWwindow *)window),
            };
            return vkCreateXlibSurfaceKHR(instance, &createInfo, pAllocator, pSurface);
        }
#endif

#ifdef VK_USE_PLATFORM_WAYLAND_KHR
        case Window::Type::Wayland:
        {
            VkWaylandSurfaceCreateInfoKHR createInfo{
                .sType   = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR,
                .pNext   = nullptr,
                .flags   = 0,
                .display = glfwGetWaylandDisplay(),
                .surface = glfwGetWaylandWindow((GLFWwindow *)window),
            };
            return vkCreateWaylandSurfaceKHR(instance, &createInfo, pAllocator, pSurface);
        }
#endif
        default:
            return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

}

}
