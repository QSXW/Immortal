#include "OpenGLSample.h"

using namespace Immortal;

int main()
{
    LOG::Setup();
    Render::Set(Render::Type::OpenGL);
    std::unique_ptr<Application> app{ new OpenGLSample() };

    app->Run();

    return 0;
}
