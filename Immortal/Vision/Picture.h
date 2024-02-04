#pragma once

#include "Core.h"
#include "Graphics/Format.h"
#include "Shared/IObject.h"
#include "Memory/MemoryResource.h"

#include <functional>

namespace Immortal
{
namespace Vision
{

enum class PictureMemoryType
{
    System,
    Device
};

class Picture;

class IMMORTAL_API SharedPictureData : public IObject
{
public:
	  friend class Picture;
	  SL_SWAPPABLE(SharedPictureData)

public:
	  SharedPictureData(Format format = Format::None, uint32_t width = 0, uint32_t height = 0, uint32_t stride = 0, bool allocate = false, MemoryResource *memoryResource = nullptr);

    ~SharedPictureData();

    void SetRelease(std::function<void(void *)> &&func);

    void Swap(SharedPictureData &other);

protected:
    uint8_t                     *data[4];
    uint32_t                     stride[4];
    Format                       format;
    uint32_t                     width;
    uint32_t                     height;
    float                        timestamp;
    PictureMemoryType            memoryType;
    std::function<void(void *)>  release;
    MemoryResource              *memoryResource;
};

class IMMORTAL_API Picture
{
public:
	Picture();

	Picture(uint32_t width, uint32_t height, Format format, bool allocated = false);

    template <class T>
    Picture(T width, T height, Format format, bool allocated = false) :
	    Picture{ (uint32_t)width, (uint32_t)height, format, allocated }
    {

    }

    operator bool() const
    {
		return !!shared;
    }

    void SetRelease(std::function<void(void*)> &&func)
    {
        shared->SetRelease(std::move(func));
    }

    auto &GetData(size_t index = 0) const
    {
        return shared->data[index];
    }

    auto &operator[](size_t index) const
    {
		return shared->data[index];
    }

    template <class T>
    void SetDataAt(size_t index, const T *data)
    {
		shared->data[index] = (uint8_t *)data;
    }

    template <class T>
    void SetData(const T *data)
    {
		SetDataAt(0, data);
    }

    PictureMemoryType GetMemoryType() const
    {
		return shared->memoryType;
    }

    void SetMemoryType(PictureMemoryType type) const
    {
		shared->memoryType = type;
    }

    void SetStride(size_t index, uint32_t stride) const
    {
		shared->stride[index] = stride;
    }

    uint32_t GetStride(size_t index) const
    {
        return shared->stride[index];
    }

    const Format &GetFormat() const
    {
        return shared->format;
    }

    void SetFormat(const Format &format)
    {
		shared->format = format;
    }

    const uint32_t &GetWidth() const
    {
        return shared->width;
    }

    void SetWidth(uint32_t width)
    {
		shared->width = width;
    }

    const uint32_t &GetHeight() const
    {
        return shared->height;
    }

    void SetHeight(uint32_t height)
    {
		shared->height = height;
    }

    const float &GetTimestamp() const
    {
		return shared->timestamp;
    }

    void SetTimestamp(float timestamp) const
    {
		shared->timestamp = timestamp;
    }

protected:
    Ref<SharedPictureData> shared;
};

}

using Picture = Vision::Picture;

}
