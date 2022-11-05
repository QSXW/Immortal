#pragma once

#include <Immortal.h>
#include "RenderLayer.h"

using namespace Immortal;

class VulkanSample : public Application
{
public:
    VulkanSample() : Application({ "Immortal Editor", 1920, 1080 })
    {
        auto renderLayer = new RenderLayer(Vector2{ 1920, 1080 }, "Debug Layer for Render");
        PushLayer(renderLayer);
    }

    ~VulkanSample()
    {

    }
};
