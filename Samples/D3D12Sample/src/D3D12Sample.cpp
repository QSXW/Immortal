#include "D3D12Sample.h"

int main()
{
    LOG::Init();
    {
        Render::Set(Render::Type::D3D12);
        std::unique_ptr<Application> app{ new D3D12Sample() };
        app->Run();
    }
    LOG::Release();

    return 0;
}
