#include "ShellItemImage.h"

#ifdef _WIN32
#include <wrl/client.h>
#include <shobjidl_core.h>
#endif

namespace Immortal
{
namespace Vision
{


void FlipBGRAImageVertical(uint8_t *_dst, int dstStride, const uint8_t *_src, int srcStride, int height)
{
    for (int y = 0; y < height; y++)
    {
		const uint8_t *src = &_src[y * srcStride];
		uint8_t       *dst = &_dst[(height - 1 - y) * dstStride];
		memcpy(dst, src, srcStride);
    }
}

ShellItemImage::ShellItemImage()
{

}

ShellItemImage::~ShellItemImage()
{

}

CodecError ShellItemImage::Decode(const CodedFrame &codedFrame)
{
#ifdef _WIN32
	using Microsoft::WRL::ComPtr;

	auto data = codedFrame.GetData();

	ComPtr<IShellItem> pShellItem;
	HRESULT hr = SHCreateItemFromParsingName((const wchar_t *)data, NULL, IID_PPV_ARGS(&pShellItem));
	if (FAILED(hr))
	{
		LOG_ERROR("Error when create shell item!");
		return CodecError::FailedToCallDecoder;
	}

	ComPtr<IShellItemImageFactory> pImageFactory;
	hr = pShellItem->QueryInterface(IID_PPV_ARGS(&pImageFactory));
	if (FAILED(hr))
	{
		LOG_ERROR("Error when query shell item image factory!");
		return CodecError::FailedToCallDecoder;
	}

	SIZE size = { 256, 256 };
	HBITMAP hBitmap = NULL;

	bool flip = false;
	hr = pImageFactory->GetImage(size, SIIGBF_THUMBNAILONLY, &hBitmap);
	if (FAILED(hr))
	{
		flip = true;
		hr = pImageFactory->GetImage(size, SIIGBF_ICONONLY, &hBitmap);
	}

	if (SUCCEEDED(hr) && hBitmap != NULL)
	{
		BITMAP bm = {0};
		GetObjectW(hBitmap, sizeof(bm), &bm);
		picture = Picture{bm.bmWidth, bm.bmHeight, Format::BGRA8, flip };

		if (flip)
		{
			FlipBGRAImageVertical(picture.GetData(), picture.GetStride(), (const uint8_t *) bm.bmBits, bm.bmWidthBytes, bm.bmHeight);
			DeleteObject(hBitmap);
		}
		else
		{
			 picture.SetDataAt(0, bm.bmBits);
			 picture.SetRelease([=](void *) { DeleteObject(hBitmap); });
		}
	}
	else
	{
		LOG_ERROR("Error when get image!");
		return CodecError::CorruptStream;
	}
	
	return CodecError::Success;

#else
	return CodecError::FailedToCallDecoder;
#endif
}

}
}
