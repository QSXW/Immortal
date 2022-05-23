/**
 * Copyright (C) 2021-2022, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

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
