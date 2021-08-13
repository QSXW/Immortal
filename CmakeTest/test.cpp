#include <iostream>
#include <memory>

#include "Immortal.h"

#include <glm/glm.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <assimp/DefaultLogger.hpp>
#include <assimp/LogStream.hpp>

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>

#include <Immortal.h>

#include <Platform/Vulkan/vkcommon.h>

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/attrdefs.h>

int main()
{
    
    auto domain = mono_jit_init("Immortal");

    glm::vec4 v;
    std::shared_ptr<spdlog::logger> log;

    Assimp::Importer importer;

    VkInstanceCreateInfo appinfo = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
    appinfo.pNext = nullptr;

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = nullptr;

    createInfo.enabledExtensionCount = glfwExtensionCount;
    createInfo.ppEnabledExtensionNames = glfwExtensions;

    VkInstance handle;
    vkCreateInstance(&appinfo, nullptr, &handle);

    auto img = cv::imread("C:/SDK/Assets/jpeg/wallhaven-1kx8k9.jpg", cv::IMREAD_COLOR);

    std::cout << img.rows << std::endl;
    system("pause");
    return 0;
}