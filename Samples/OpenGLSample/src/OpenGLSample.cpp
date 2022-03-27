#include "OpenGLSample.h"

using namespace Immortal;

int main()
{
    LOG::Init();
    {
        Render::Set(Render::Type::OpenGL);
        std::unique_ptr<Application> app{ new OpenGLSample() };
        app->Run();
    }
    LOG::Release();

    return 0;
}
