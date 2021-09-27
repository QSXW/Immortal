#include "D3D12Sample.h"

#include "Immortal.h"
#include "Framework/Main.h"

using namespace Immortal;

class D3D12Sample : public Application
{
public:
    D3D12Sample() : Application({ U8("D3D12 Sample (Graphics API: DirectX 12)"), 1920, 1080 })
    {

    }

    ~D3D12Sample()
    {

    }

private:
    
};

Immortal::Application* Immortal::CreateApplication()
{
    Render::Set(Render::Type::D3D12);
    return new D3D12Sample();
}
