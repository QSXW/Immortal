#pragma once

#include "Core.h"
#include "Common/ByteStream.h"

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
	IFD       = 13,
	Invalid   = 0xFFFF,
};

#define IFD0         0
#define IFD1         1
#define IFD2         2
#define SubIFD0      3
#define SubIFD1      4
#define SubIFD2      5
#define SubIFD3      6
#define IFD_EXIF     7
#define IFD_INTEROP  8
#define IFD_GPS      9
#define IFD_MAKENOTE 10
#define IFDPRIVATE   11

//enum class IFDTag : uint16_t
//{
//	Invalid = 0xFFFF,
//};

union EntryValue
{
	void     *ptr;
	int64_t  *sint;
	uint64_t *uint;
	double   *dbl;
	char     *str;
	uint8_t  *ubytes;
	int8_t   *sbytes;
	uint16_t *ushorts;
	int16_t *shorts;
	TRational<uint32_t> *rational;
	TRational<int32_t> *srational;
};

class IFDEntry
{
public:
	SL_ENABLE_COPY(IFDEntry)
	SL_ENABLE_MOVE(IFDEntry)

	IFDEntry();

	IFDEntry(ByteStream &bs);

	IFDEntry(ByteStream &bs, bool be);

    ~IFDEntry();

	void ReadValues(ByteStream &bs, bool be = false);

    template <class T>
	bool ReadValue(ByteStream &bs, bool be)
	{
		const uint8_t *src = nullptr;

		auto size = sizeof(T) * count;
		values.resize(size);
		
		v.ptr = (void *) values.data();
		if (size <= 4)
		{
			if (be)
			{
				const uint8_t raw[4] = {
					(uint8_t)(offset >> 24),
					(uint8_t)(offset >> 16),
					(uint8_t)(offset >> 8),
					(uint8_t)offset,
				};
				if constexpr (sizeof(T) == 1)
				{
					memcpy(values.data(), raw, size);
				}
				else
				{
					for (size_t i = 0; i < count; i++)
					{
						uint8_t *dst = values.data() + i * sizeof(T);
						for (size_t j = 0; j < sizeof(T); j++)
						{
							dst[j] = raw[i * sizeof(T) + sizeof(T) - j - 1];
						}
					}
				}
			}
			else
			{
				memcpy(values.data(), &offset, size);
			}
		}
		else if (size > bs.get_bytes_left())
		{
			return false;
		}
		else if constexpr (std::is_same_v<T, uint8_t>)
		{
			bs.get_buffer(values.data(), size);
		}
		else
		{
			if (be)
			{
				for (size_t i = 0; i < count; i++)
				{
					((T *)v.ptr)[i] = bs.get_be<T>();
				}
			}
			else
			{
				bs.get_buffer(values.data(), size);
			}
		}

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
		return count;
    }

    uint32_t GetOffset() const
	{
		return offset;
	}

    template <class T>
    T GetValue() const
    {
		return *(T *)values.data();
    }

    const char *GetString() const
	{
		return (const char *)values.data();
    }

	static size_t size()
	{
		return 12;
	}

	IFDEntry(const IFDEntry &other) :
	    tag{other.tag},
	    type{other.type},
	    count{other.count},
	    offset{other.offset},
	    values{other.values},
	    v{}
	{
		v.ptr = values.data();
	}

	void Swap(IFDEntry &other)
	{
		std::swap(tag,    other.tag   );
		std::swap(type,   other.type  );
		std::swap(count,  other.count );
		std::swap(offset, other.offset);
		std::swap(values, other.values);
		std::swap(v.ptr,  other.v.ptr );
	}

public:
	uint16_t tag;
	IFDType  type;
	uint32_t count;
	uint32_t offset;
	std::vector<uint8_t> values;
	EntryValue v;
};

enum class ExifFlags
{
	DisabledRecursive = BIT(0),
};
SL_ENABLE_BITWISE_OPERATOR(ExifFlags)

class ExifReader
{
public:
	SL_ENABLE_MOVE(ExifReader)

public:
	ExifReader();

	ExifReader(const uint8_t *data, size_t size, ExifFlags flags = {});

	CodecError ParseHeader(ByteStream &bs, uint32_t &offsetToFirstIDF, bool &be);

	CodecError ParseIFD(ByteStream &bs, int startIFDindex, uint32_t offsetOfNextIFD);

	CodecError ParseIFD(ByteStream &bs, int index, uint32_t *offsetOfNextIFD, bool be = false);

	CodecError MMParseIFD(ByteStream &bs, int startIFDindex, uint32_t *offsetOfNextIFD);

public:
	const std::pmr::unordered_map<uint16_t, IFDEntry> &GetIFD(int index) const
	{
		if (index >= SL_ARRAY_LENGTH(IFD))
		{
			return {};
		}

		return IFD[index];
	}

	template <class T>
	T GetValue(int index, uint16_t tag) const
	{
		auto &ifd = GetIFD(index);

		auto it = ifd.find(tag);
		if (it != ifd.end())
		{
			auto &[tag, entry] = *it;
			return entry.GetValue<T>();
		}

		return {};
	}

	String GetString(const std::pmr::unordered_map<uint16_t, IFDEntry> &ifd, uint16_t tag)
	{
		auto it = ifd.find(tag);
		if (it == ifd.end())
		{
			return {};
		}

		auto &[t, entry] = *it;
		auto &v = entry.v;
		switch (entry.type)
		{
			case IFDType::ASCII:
				if (entry.values.empty())
				{
					return {};
				}
				return String{ entry.v.str, StringEncoding::UTF8 };

			case IFDType::SHORT:
				if (entry.count == 1)
				{
					return String{ std::to_string((int16_t)*v.shorts), StringEncoding::ASCII };
				}
				return {};

			case IFDType::LONG:
				if (entry.count == 1)
				{
					return String{ std::to_string((uint32_t)*v.uint), StringEncoding::ASCII };
				}
				return {};

			case IFDType::RATIONAL:
				if (entry.count == 1)
				{
					auto &r = *v.rational;
					return String{ std::to_string(r.numerator) + "/" + std::to_string(r.dominator), StringEncoding::ASCII };
				}
				return {};

			case IFDType::UNDEFINED:
				return {};

			case IFDType::SSHORT:
				if (entry.count == 1)
				{
					return String{ std::to_string((uint32_t)*v.shorts), StringEncoding::ASCII };
				}
				return {};

			case IFDType::SLONG:
				if (entry.count == 1)
				{
					return String{ std::to_string((int32_t)*v.sint), StringEncoding::ASCII };
				}
				return {};

			case IFDType::SRATIONAL:
				if (entry.count == 1)
				{
					auto &r = *v.srational;
					return String{ std::to_string(r.numerator) + "/" + std::to_string(r.dominator), StringEncoding::ASCII };
				}
				return {};

			case IFDType::FLOAT:
				if (entry.count == 1)
				{
					return String{ std::to_string(*(float *)v.dbl), StringEncoding::ASCII };
				}
				return {};

			case IFDType::DOUBLE:
				if (entry.count == 1)
				{
					return String{ std::to_string(*v.dbl), StringEncoding::ASCII };
				}
				return {};

			default:
				break;
		}
		return {};
	}

	void Swap(ExifReader &other)
	{
		std::swap(IFD,   other.IFD  );
		std::swap(flags, other.flags);
		std::swap(be,    other.be   );
	}

protected:
	std::pmr::unordered_map<uint16_t, IFDEntry> IFD[IFDPRIVATE + 1];

	ExifFlags flags;

	bool be;
};

}
}
