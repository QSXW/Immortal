#include "VulkanSample.h"

int main()
{
    LOG::Init();

    {
        Render::Set(Render::Type::Vulkan);
        std::unique_ptr<Application> app{ new VulkanSample() };
        app->Run();
    }

    LOG::Release();

    return 0;
}
