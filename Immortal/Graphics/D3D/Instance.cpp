#include "Instance.h"

namespace Immortal
{
namespace D3D
{

Instance::Instance() :
    handle{},
    dxgiLibrary{},
    dxgiDebugLibrary{},
    CreateDXGIFactory2{}
{
	dxgiLibrary = LoadLibraryA("dxgi.dll");
	if (!dxgiLibrary)
	{
		LOG::ERR("Failed to load dxgi.dll");
	}

	uint32_t dxgiFactoryFlags = 0;

#if _DEBUG
	dxgiDebugLibrary = LoadLibraryA("dxgidebug.dll");
	if (dxgiDebugLibrary)
	{
		dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
	}
#endif

	CreateDXGIFactory2 = (PFN_CreateDXGIFactory2)GetProcAddress(dxgiLibrary, "CreateDXGIFactory2");
	Check(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&handle)));
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

#ifdef _DEBUG
	if (dxgiDebugLibrary)
	{
		ComPtr<IDXGIDebug> dxgiDebug;
		auto __DXGIGetDebugInterface = (decltype(&DXGIGetDebugInterface)) ::GetProcAddress(dxgiDebugLibrary, "DXGIGetDebugInterface");
		Check(__DXGIGetDebugInterface(IID_PPV_ARGS(&dxgiDebug)));
		Check(dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL));

		FreeLibrary(dxgiDebugLibrary);
		dxgiDebugLibrary = nullptr;
	}
#endif
}

int Instance::EnumeratePhysicalDevice(uint32_t *numPhysicalDevice, SuperPhysicalDevice **ppPhysicalDevice)
{
	if (physicalDevices.empty())
	{
		for (int adapterIndex = 0;; adapterIndex++)
		{
			ComPtr<IDXGIAdapter1> pAdapter = nullptr;
			if (handle->EnumAdapters1(adapterIndex, &pAdapter) == DXGI_ERROR_NOT_FOUND)
			{
				break;
			}

			physicalDevices.resize(adapterIndex + 1);
			if (!physicalDevices[adapterIndex])
			{
				physicalDevices[adapterIndex] = new PhysicalDevice{ this, adapterIndex };
			}
		}
	}

	*numPhysicalDevice = (uint32_t)physicalDevices.size();
	if (!ppPhysicalDevice)
	{
		return 0;
	}
	for (size_t i = 0; i < physicalDevices.size(); i++)
	{
		ppPhysicalDevice[i] = physicalDevices[i];
	}

	return 0;
}

void Instance::EnumerateAdapter(uint32_t deviceId, IDXGIAdapter1 **ppAdapter)
{
	Check(handle->EnumAdapters1(deviceId, ppAdapter));
}

}
}
