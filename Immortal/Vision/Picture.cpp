#include "Picture.h"
#include "Memory/MemoryResource.h"
#include "Graphics/Texture.h"

namespace Immortal
{
namespace Vision
{

SharedPictureData::SharedPictureData(Format format, uint32_t width, uint32_t height, uint32_t _stride, bool allocate, MemoryResource *memoryResource) :
    data{},
    stride{},
    format{ format },
    width{ width },
    height{ height },
    timestamp{},
    memoryType{},
    release{},
    memoryResource{ memoryResource },
    allocator{}
{
	stride[0] = _stride;
    if (allocate)
	{
		stride[0] = SLALIGN(stride[0], TextureAlignment);
        if (memoryResource)
        {
            data[0] = (uint8_t *)memoryResource->Allocate();
            SetRelease([=, this] (void *ptr) {
				this->memoryResource->Release(ptr);
            });
        }
        else
        {
			size_t size = 0;
            if (format.IsType(Format::YUV))
            {
				SamplingFactor factors[SamplingFactor::kMaxSublayer];
				GetSamplingFactor(format, factors);

                size_t pixelShift = format.IsType(Format::HightBitDepth) ? 1 : 0;
                size_t offsets[3] = {};
                for (int i = 0; i < SL_ARRAY_LENGTH(offsets); i++)
                {
					offsets[i] = size;
					size_t x = width  << pixelShift;
					size_t y = SLALIGN(height, i > 0 ? 2 : 1) >> factors[i].y;

					stride[i] = SLALIGN(x >> factors[i].x, 8);
					size += stride[i] * y;
                }

				data[0] = allocator.allocate(size);
                for (int i = 1; i < SL_ARRAY_LENGTH(offsets); i++)
                {
					data[i] = data[0] + offsets[i];
                }
            }
            else
            {
				size = stride[0] * height;
				data[0] = allocator.allocate(size);
            }
      
			SetRelease([=, this](void *_ptr) {
				allocator.deallocate((uint8_t *)_ptr);
            });
        }
    }
}

SharedPictureData::SharedPictureData(Texture *texture) :
    SharedPictureData{texture->GetFormat(), texture->GetWidth(), texture->GetHeight()}
{
	data[0] = (uint8_t *)texture;
}

SharedPictureData::~SharedPictureData()
{
	if (release)
	{
		release(data[0]);
	}
}

void SharedPictureData::SetRelease(std::function<void(void *)> &&func)
{
	release = std::move(func);
}

void SharedPictureData::Swap(SharedPictureData &other)
{
	std::swap_ranges(data,   data   + SL_ARRAY_LENGTH(data),   other.data  );
	std::swap_ranges(stride, stride + SL_ARRAY_LENGTH(stride), other.stride);
	std::swap(format,         other.format        );
	std::swap(width,          other.width         );
	std::swap(height,         other.height        );
    std::swap(memoryType,     other.memoryType    );
	std::swap(release,        other.release       );
	std::swap(memoryResource, other.memoryResource);
}

Picture::Picture() :
    shared{}
{

}

Picture::Picture(uint32_t width, uint32_t height, Format format, bool allocated) :
    shared{ new SharedPictureData{ format, width, height, (uint32_t)(width * format.GetTexelSize()), allocated }}
{

}

Picture::Picture(Texture *texture) :
    shared{ new SharedPictureData{ texture }}
{

}

}
}
