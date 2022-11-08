/**
 * Copyright (C) 2021-2022, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#include "RenderLayer.h"

using namespace Immortal;

class ImmortalEditor : public Application
{
public:
    ImmortalEditor() : Application({ "Immortal Editor", 1920, 1080 })
    {
        auto renderLayer = new RenderLayer(Vector2{ 1920, 1080 }, "Debug Layer for Render");
        PushLayer(renderLayer);
    }

    ~ImmortalEditor()
    {

    }
};

int main(int argc, char **argv)
{
    LOG::Init();

    try
    {
        Render::Type type = Render::Type::Vulkan;

        Arguments args{ argc, argv };
        auto &api = args["api"];
        if (api == "vk")
        {
            type = Render::Type::Vulkan;
        }
        else if (api == "ogl")
        {
            type = Render::Type::OpenGL;
        }
        else if (api == "d3d11")
        {
            type = Render::Type::D3D11;
        }
        else if (api == "d3d12")
        {
            type = Render::Type::D3D12;
        }

        int deviceId = AUTO_DEVICE_ID;
        const std::string &device = args["device"];
        if (!device.empty())
        {
            deviceId = std::atoi(device.c_str());
        }

        Render::Set(type, deviceId);
        URef<Application> app{ new ImmortalEditor() };
        app->Run();
    }
    catch (const std::exception &e)
    {
        LOG::FATAL("Critical Error Detected: {}", e.what());
        LOG::FATAL("The program {} will be exited!", argv[0]);
    }

    LOG::Release();

    return 0;
}
