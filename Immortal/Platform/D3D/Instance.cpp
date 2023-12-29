#include "Instance.h"

namespace Immortal
{
namespace D3D
{

Instance::Instance() :
    handle{},
    dxgiLibrary{},
    CreateDXGIFactory2{}
{
	dxgiLibrary = LoadLibraryA("dxgi.dll");
	if (!dxgiLibrary)
	{
		LOG::ERR("Failed to load dxgi.dll");
	}

	uint32_t dxgiFactoryFlags = 0;

#if _DEBUG
	dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif

	CreateDXGIFactory2 = (PFN_CreateDXGIFactory2)GetProcAddress(dxgiLibrary, "CreateDXGIFactory2");
	Check(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&handle)), "Failed to create DXGI Factory");
}

Instance::~Instance()
{
	physicalDevices.clear();
	handle.Reset();
	if (dxgiLibrary)
	{
		FreeLibrary(dxgiLibrary);
		dxgiLibrary = nullptr;
	}
}

void Instance::EnumerateAdapter(uint32_t deviceId, IDXGIAdapter1 **ppAdapter)
{
	Check(handle->EnumAdapters1(deviceId, ppAdapter));
}

}
}
