#include "Picture.h"
#include "Memory/MemoryResource.h"

namespace Immortal
{
namespace Vision
{

SharedPictureData::SharedPictureData(Format format, uint32_t width, uint32_t height, uint32_t stride, bool allocate, MemoryResource *memoryResource) :
    data{},
    stride{},
    format{ format },
    width{ width },
    height{ height },
    timestamp{},
    memoryType{},
    release{},
    memoryResource{ memoryResource }
{
    if (allocate)
    {
        if (memoryResource)
        {
            data[0] = (uint8_t *)memoryResource->Allocate();
            SetRelease([=] (void *data) {
				this->memoryResource->Release(data);
            });
        }
        else
        {
            data[0] = new uint8_t[width * height * format.GetTexelSize()];
			SetRelease([](void *data) {
				delete[] data;
            });
        }
    }
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
	std::swap_ranges(data,   data   + sizeof(data),   other.data  );
	std::swap_ranges(stride, stride + sizeof(stride), other.stride);
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
    shared{ new SharedPictureData{ format, width, height, width, allocated }}
{
    
}

}
}
