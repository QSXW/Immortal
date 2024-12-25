/**
 * Copyright (C) 2023, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#include "WindowCapture.h"
#include "Device.h"
#include "Texture.h"

namespace Immortal
{

namespace D3D11
{

WindowCapture::WindowCapture(Device *device) :
	device{ device }
{
	IDXGIAdapter1 *adapter = device->GetAdapter();

	ComPtr<IDXGIOutput> pOutput;
	std::vector<ComPtr<IDXGIOutput>> outputs;
	for (uint32_t i = 0; adapter->EnumOutputs(i, &pOutput) != DXGI_ERROR_NOT_FOUND; i++)
	{
		outputs.push_back(pOutput);
	}
	if (outputs.empty())
	{
		return;
	}
	DX_CHECK(outputs[0]->QueryInterface(output.GetAddressOf()));

	DXGI_FORMAT supportedFormats[] = {
		DXGI_FORMAT_R8G8B8A8_UNORM
	};

	SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE);
	DX_CHECK(output->DuplicateOutput1(*device, 0, SL_ARRAY_LENGTH(supportedFormats), supportedFormats, &outputDuplication));
}

WindowCapture::~WindowCapture()
{

}

SuperTexture *WindowCapture::AcquireNextFrame(CaptureFrameInfo *pFrameInfo)
{
	DXGI_OUTDUPL_FRAME_INFO outputFrameInfo{};
	ComPtr<IDXGIResource> resource;
	DX_CHECK(outputDuplication->AcquireNextFrame(INFINITE, &outputFrameInfo, resource.GetAddressOf()));

	pFrameInfo->accumulatedFrames = outputFrameInfo.AccumulatedFrames;

	ComPtr<ID3D11Texture2D> texture;
	DX_CHECK(resource->QueryInterface(texture.GetAddressOf()));

	auto ret = new Texture{ device, texture };
	DX_CHECK(outputDuplication->ReleaseFrame());

	return ret;
}

}

}
