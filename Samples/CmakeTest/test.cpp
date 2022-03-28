#include <iostream>
#include <memory>

#include <Immortal.h>
#include <Platform/Vulkan/Common.h>

#include <glm/glm.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#if HAVE_ASSIMP
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include <assimp/DefaultLogger.hpp>
#include <assimp/LogStream.hpp>
#endif

#if HAVE_OPENCV
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#endif

void TestRef()
{
    using namespace Immortal;
    class A : public IObject
    {

    };

    Ref<A> b = nullptr;

    {
        Ref<A> a = new A;
        b = a;
    }

    Ref<A> c = new A;
    c = b;

    {
        MonoRef<A> a = new A;
        a.Reset(new A);
    }
}

int main()
{
    glm::vec4 v;
    std::shared_ptr<spdlog::logger> log;

    VkInstanceCreateInfo appinfo = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
    appinfo.pNext = nullptr;

    VkApplicationInfo appInfo{};
    appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName   = "CMake Test";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName        = "CMake Test Engine";
    appInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion         = VK_API_VERSION_1_0;

    volkInitialize();
    VkInstanceCreateInfo createInfo{};
    createInfo.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.enabledExtensionCount   = 0;
    createInfo.enabledLayerCount       = 0;
    createInfo.ppEnabledExtensionNames = nullptr;
    createInfo.ppEnabledLayerNames     = nullptr;
    createInfo.pApplicationInfo        = &appInfo;

    VkInstance handle;
    vkCreateInstance(&createInfo, nullptr, &handle);

#if HAVE_ASSIMP
    Assimp::Importer importer;
#endif

#if HAVE_OPENCV
    auto img = cv::imread("Assets/Icon/terminal.png", cv::IMREAD_COLOR);
    std::cout << img.rows << std::endl;
#endif

    TestRef();

    return 0;
}
