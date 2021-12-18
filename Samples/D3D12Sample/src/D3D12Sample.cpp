#include "D3D12Sample.h"
using namespace Immortal;

int main()
{
    LOG::Setup();
    Render::Set(Render::Type::D3D12);

    std::unique_ptr<Application> app{ new D3D12Sample() };

    app->Run();

    return 0;
}
