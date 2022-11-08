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
        Render::Set(Render::Type::Vulkan);
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
