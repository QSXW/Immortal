#pragma once

#include "Immortal.h"
#include "RenderLayer.h"

class OpenGLSample : public Application
{
public:
    OpenGLSample()
    {
        PushLayer(new RenderLayer());
    }

    ~OpenGLSample()
    {

    }
};
