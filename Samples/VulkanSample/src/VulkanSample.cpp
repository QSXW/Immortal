#include "VulkanSample.h"

int main(int argc, char **argv)
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
