#include "Picture.h"
#include "Memory/MemoryResource.h"
#include "Graphics/Texture.h"

namespace Immortal
{
namespace Vision
{

static BaseProperty *AllocateSideData(PropertyType type)
{
	size_t size = 0;
    switch (type)
    {
    case PropertyType::DisplayOrientation:
        size = sizeof(DisplayOrientation);
	    break;

    case PropertyType::ICCProfile:
		size = sizeof(ICCProfileProperty);
		break;

    default:
	    return nullptr;
    }
    
    auto data = malloc(size);
	if (data)
	{
		memset(data, 0, size);
	}

	return (BaseProperty *)data;
}

SharedPictureData::SharedPictureData(Format format, uint32_t width, uint32_t height, uint32_t _stride, bool allocate, MemoryResource *memoryResource, std::initializer_list<PropertyType> &&types) :
    data{},
    stride{},
    format{ format },
    width{ width },
    height{ height },
    sampleRate{},
    flags{},
    timestamp{},
    timebase{},
    sampleAspectRatio{ 1, 1 },
    memoryType{},
    colorSpace{},
    trc{},
    release{},
    memoryResource{ memoryResource },
    allocator{}
{
	stride[0] = _stride;
	if (allocate)
	{
		stride[0] = SLALIGN(stride[0], TextureAlignment);
	}

	if (memoryResource && allocate)
    {
        data[0] = (uint8_t *)memoryResource->Allocate();
        SetRelease([=, this] (void *ptr) {
			this->memoryResource->Release(ptr);
        });
    }
    else
    {
		size_t size = 0;
        if (format.IsType(Format::YUV) && !format.IsType(Format::YUYV))
        {
			SamplingFactor factors[SamplingFactor::kMaxSublayer];
			GetSamplingFactor(format, factors);

            size_t pixelShift = format.IsType(Format::HightBitDepth) ? 1 : 0;
            size_t offsets[3] = {};
            for (int i = 0; i < SL_ARRAY_LENGTH(offsets); i++)
            {
				offsets[i] = size;
				size_t x = size_t(width) << pixelShift;
				size_t y = SLALIGN(height, i > 0 ? 2 : 1) >> factors[i].y;

				stride[i] = SLALIGN(x >> factors[i].x, 8);
				size += stride[i] * y;
            }

            if (format.IsType(Format::NV))
            {
				stride[1] += stride[2];
				stride[2]  = 0;
            }

            if (allocate)
			{
				data[0] = allocator.allocate(size);

                for (int i = 1; stride[i]; i++)
                {
				    data[i] = data[0] + offsets[i];
                }
			}
        }
		else if (allocate)
        {
			size = stride[0] * height;
			data[0] = allocator.allocate(size);
        }
        
        if (allocate)
		{
			SetRelease([=, this](void *_ptr) {
				allocator.deallocate((uint8_t *) _ptr);
			});
		}
    }

    properties.reserve(types.size());
	for (auto &type : types)
    {
		AllocateProperty(type);
    }
}

SharedPictureData::SharedPictureData(Texture *texture) :
    SharedPictureData{texture->GetFormat(), texture->GetWidth(), texture->GetHeight()}
{
	data[0] = (uint8_t *)texture;
}

SharedPictureData::~SharedPictureData()
{
    for (auto &prop : properties)
    {
		free(prop);
    }
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
	std::swap(sampleRate,     other.sampleRate    );
	std::swap(timestamp,      other.timestamp     );
	std::swap(timebase,       other.timebase      );
	std::swap(sampleAspectRatio, other.sampleAspectRatio);
    std::swap(memoryType,     other.memoryType    );
	std::swap(release,        other.release       );
	std::swap(memoryResource, other.memoryResource);
    std::swap(colorSpace,     other.colorSpace    );
	std::swap(trc,            other.trc           );
    std::swap(flags,          other.flags         );
}

BaseProperty *SharedPictureData::AllocateProperty(PropertyType type)
{
	BaseProperty *p = AllocateSideData(type);
	p->type = type;
	properties.emplace_back(p);

    return p;
}

BaseProperty *SharedPictureData::GetProperty(PropertyType type) const
{
	for (auto &prop : properties)
    {
        if (prop->type == type)
        {
			return prop;
        }
    }

    return nullptr;
}

Picture::Picture() :
    shared{}
{

}

Picture::Picture(uint32_t width, uint32_t height, Format format, bool allocated, std::initializer_list<PropertyType> &&types) :
    shared{ new SharedPictureData{ format, width, height, (uint32_t)(width * format.GetTexelSize()), allocated, nullptr, std::move(types) }}
{

}

Picture::Picture(Texture *texture) :
    shared{ new SharedPictureData{ texture }}
{

}

}
}
