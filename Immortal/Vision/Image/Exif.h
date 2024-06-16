#include "Core.h"

namespace Immortal
{
namespace Vision
{

using IFDMemoryResource = std::pmr::monotonic_buffer_resource;

template <class T>
struct TRational
{
	T numerator;
	T dominator;
};

enum class IFDType : uint16_t
{
	BYTE      = 1,
	ASCII     = 2,
    SHORT     = 3,
    LONG      = 4,
    RATIONAL  = 5,
    SBYTE     = 6,
	UNDEFINED = 7,
	SSHORT    = 8,
	SLONG     = 9,
	SRATIONAL = 10,
	FLOAT     = 11,
	DOUBLE    = 12,
	Invalid   = 0xFFFF,
};

#define IFD0       0
#define IFD1       1
#define IFD2       2
#define SubIFD0    3
#define SubIFD1    4
#define SubIFD2    5
#define IFDPRIVATE 6

enum class IFDTag : uint16_t
{
	Invalid = 0xFFFF,
};

class IFDEntry
{
public:
	IFDEntry();

	IFDEntry(IFDMemoryResource *memoryResource, const uint8_t *data, size_t offset, size_t size);

    IFDEntry(IFDMemoryResource *memoryResource, const uint8_t *data, size_t offset, size_t size, bool isBigEndian);

    ~IFDEntry();

	void ReadValues(const uint8_t *data, size_t end);

    template <class T>
	bool ReadValue(const uint8_t *data, size_t end)
	{
		const uint8_t *src = nullptr;

        auto size = sizeof(T) * length;
		if (size <= 4)
		{
			src = (const uint8_t *)&offset;
		}
		else
		{
			if (size + offset >= end)
			{
				return false;
			}
			src = &data[offset];
		}

		value.resize(size);
		memcpy(value.data(), src, value.size());

        return true;
	}

    uint16_t GetTag() const
    {
		return (uint16_t)tag;
    }

    uint16_t GetType() const
	{
		return (uint16_t)type;
	}

    uint32_t GetLength() const
    {
		return length;
    }

    uint32_t GetOffset() const
	{
		return offset;
	}

    template <class T>
    T GetValue() const
    {
		return *(T *)value.data();
    }

    const char *GetString() const
	{
		return (const char *)value.data();
    }

	static size_t size()
	{
		return 12;
	}

protected:
	IFDTag   tag;
	IFDType  type;
	uint32_t length;
	uint32_t offset;
	std::pmr::vector<uint8_t> value;
};

class ExifReader
{
public:
	ExifReader(const uint8_t *data, size_t size);

	CodecError ParseHeader(const uint8_t *imageFileHeader, size_t size);

	CodecError ParseIFD(const uint8_t *imageFileHeader, size_t size, int startIFDindex, uint32_t offsetOfNextIFD);

	CodecError ParseIFD(const uint8_t *imageFileHeader, size_t size, int index, uint32_t *offsetOfNextIFD);

	CodecError MMParseIFD(const uint8_t *imageFileHeader, size_t size, int startIFDindex, uint32_t *offsetOfNextIFD);

	CodecError ParseIFD0(const uint8_t *imageFileHeader, size_t size, const IFDEntry &entry);

	CodecError ParseIFD1(const uint8_t *imageFileHeader, size_t size, const IFDEntry &entry);

	CodecError ParseIFD2(const uint8_t *imageFileHeader, size_t size, const IFDEntry &entry);

public:
	template <class T>
	T GetValue(int IFDindex, uint16_t tag) const
	{
		if (IFDindex >= SL_ARRAY_LENGTH(IFD))
		{
			return {};
		}

		auto it = IFD[IFDindex].find(tag);
		if (it != IFD[IFDindex].end())
		{
			auto &[tag, entry] = *it;
			return entry.GetValue<T>();
		}

		return {};
	}

protected:
	std::vector<uint8_t> buffer;

	std::pmr::polymorphic_allocator<std::pair<const uint16_t, IFDEntry>> allocator;

	IFDMemoryResource bufferResource;

	uint32_t offsetToFirstIDF;

	std::pmr::unordered_map<uint16_t, IFDEntry> IFD[IFDPRIVATE + 1];

	bool isBigEndian;
};

}
}
