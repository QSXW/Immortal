#include "D3D12Sample.h"

using namespace Immortal;

int main()
{
    LOG::Init();

    auto hmodule = LoadLibraryA("D3d12SDKLayers.dll");

    D3D12::RenderContext context{ nullptr };

    return 0;
}
