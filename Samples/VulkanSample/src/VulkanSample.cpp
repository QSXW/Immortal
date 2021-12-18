#include "VulkanSample.h"

int main()
{
    LOG::Setup();
    Render::Set(Render::Type::Vulkan);

    std::unique_ptr<Application> app{ new VulkanSample() };

    app->Run();

    return 0;
}
