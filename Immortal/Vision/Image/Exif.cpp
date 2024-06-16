#include "Exif.h"

namespace Immortal
{
namespace Vision
{

template <class T>
T InterpretAs(const void *data)
{
	return ((T *) data)[0];
}

template <class T>
T MMInterpretAs(const void *data)
{
	T word = {};
	uint8_t *byte = (uint8_t *)&word;
	const uint8_t *bytes = (const uint8_t *)data;
	if constexpr (sizeof(T) == 2)
	{	
		byte[1] = bytes[0];
		byte[0] = bytes[1];
	}
	else if constexpr (sizeof(T) == 4)
	{
		byte[3] = bytes[0];
		byte[2] = bytes[1];
		byte[1] = bytes[2];
		byte[0] = bytes[3];
	}
	else if constexpr (sizeof(T) == 8)
	{
		byte[7] = bytes[0];
		byte[6] = bytes[1];
		byte[5] = bytes[2];
		byte[4] = bytes[3];
		byte[3] = bytes[4];
		byte[2] = bytes[5];
		byte[1] = bytes[6];
		byte[0] = bytes[7];
	}
	
	return word;
}

IFDEntry::IFDEntry() :
	tag{ IFDTag::Invalid },
	type{ IFDType::Invalid },
    length{},
    offset{}
{

}

IFDEntry::IFDEntry(IFDMemoryResource *memoryResource, const uint8_t *data, size_t offset, size_t size) :
    tag{ InterpretAs<uint16_t>(&data[offset]) },
    type{ InterpretAs<uint16_t>(&data[offset] + 2) },
    length{ InterpretAs<uint32_t>(&data[offset] + 4) },
    offset{ InterpretAs<uint32_t>(&data[offset] + 8) },
    value{ memoryResource }
{
	ReadValues(data, size);
}

IFDEntry::IFDEntry(IFDMemoryResource *memoryResource, const uint8_t *data, size_t offset, size_t size, bool isBigEndian) :
    tag{ MMInterpretAs<uint16_t>(&data[offset]) },
    type{ MMInterpretAs<uint16_t>(&data[offset] + 2) },
    length{ MMInterpretAs<uint32_t>(&data[offset] + 4) },
    offset{ MMInterpretAs<uint32_t>(&data[offset] + 8) },
    value{ memoryResource }
{
	ReadValues(data, size);
}

IFDEntry::~IFDEntry()
{

}

void IFDEntry::ReadValues(const uint8_t *data, size_t size)
{
	bool valid = false;
	switch (type)
	{
		case IFDType::BYTE:
		case IFDType::SBYTE:
		case IFDType::ASCII:
			valid = ReadValue<uint8_t>(data, size);
			break;

		case IFDType::SHORT:
			valid = ReadValue<uint16_t>(data, size);
			break;

		case IFDType::LONG:
			valid = ReadValue<uint32_t>(data, size);
			break;

		case IFDType::RATIONAL:
			valid = ReadValue<TRational<uint32_t>>(data, size);
			break;

		case IFDType::UNDEFINED:
			valid = ReadValue<uint8_t>(data, size);
			break;

		case IFDType::SSHORT:
			valid = ReadValue<int16_t>(data, size);
			break;

		case IFDType::SLONG:
			valid = ReadValue<int32_t>(data, size);
			break;

		case IFDType::SRATIONAL:
			valid = ReadValue<TRational<int32_t>>(data, size);
			break;

		case IFDType::FLOAT:
			valid = ReadValue<float>(data, size);
			break;

		case IFDType::DOUBLE:
			valid = ReadValue<double>(data, size);
			break;

		default:
			break;
	}

	if (!valid)
	{
		tag = IFDTag::Invalid;
	}
}

ExifReader::ExifReader(const uint8_t *data, size_t size) :
    buffer{std::move(std::vector<uint8_t>(size))},
    bufferResource{buffer.data(), buffer.size()},
    offsetToFirstIDF{},
    allocator{ &bufferResource },
    IFD{},
    isBigEndian{}
{
	for (int i = 0; i < SL_ARRAY_LENGTH(IFD); i++)
	{
		IFD[i] = std::pmr::unordered_map<uint16_t, IFDEntry>{allocator};
	}

	if (ParseHeader(data, size) != CodecError::Success)
	{
		return;
	}
		
	if (ParseIFD(data, size, IFD0, offsetToFirstIDF) != CodecError::Success)
	{
		return;
	}

	for (size_t i = IFD0; i <= IFD2; i++)
	{
		if (!IFD[i].empty())
		{
			auto it = IFD[i].find(0x014a);
			if (it != IFD[i].end())
			{
				if (ParseIFD(data, size, SubIFD0, it->second.GetValue<uint32_t>()) != CodecError::Success)
				{
					return;
				}
			}
		}
	}
}

CodecError ExifReader::ParseHeader(const uint8_t *imageFileHeader, size_t size)
{
	auto p = imageFileHeader;

	auto endian = InterpretAs<uint16_t>(imageFileHeader);
	if (endian == 0x4D4D /* MM */)
	{
		isBigEndian = true;
	}
	else if (endian != 0x4949 /* II */)
	{
		return CodecError::CorruptStream;
	}
	p += sizeof(uint16_t);

	uint16_t _42 = 0;
	if (isBigEndian)
	{
		_42 = MMInterpretAs<uint16_t>(p);
		p += sizeof(uint16_t);
		offsetToFirstIDF = MMInterpretAs<uint32_t>(p);
	}
	else
	{
		_42 = InterpretAs<uint16_t>(p);
		p += sizeof(uint16_t);
		offsetToFirstIDF = InterpretAs<uint32_t>(p);
	}

	if (_42 != 0x002a)
	{
		return CodecError::CorruptStream;
	}

	return CodecError::Success;
}

CodecError ExifReader::ParseIFD(const uint8_t *imageFileHeader, size_t size, int startIFDindex, uint32_t offsetOfNextIFD)
{
	CodecError ret = {};
	for (int index = startIFDindex; offsetOfNextIFD != 0; index++)
	{
		if (isBigEndian)
		{
			if (MMParseIFD(imageFileHeader, size, index, &offsetOfNextIFD) != CodecError::Success)
			{
				return ret;
			}
		}
		else
		{
			if (ParseIFD(imageFileHeader, size, index, &offsetOfNextIFD) != CodecError::Success)
			{
				return ret;
			}
		}
	}

	return CodecError::Success;
}

CodecError ExifReader::ParseIFD(const uint8_t *imageFileHeader, size_t size, int index, uint32_t *offsetOfNextIFD)
{
	auto p = imageFileHeader + *offsetOfNextIFD;

	int numDirectoryEntries = InterpretAs<uint16_t>(p);
	p += sizeof(uint16_t);

	while (--numDirectoryEntries >= 0)
	{
		size_t offset = p - imageFileHeader;
		IFDEntry entry = { &bufferResource, imageFileHeader, offset, size };
		p += entry.size();
		IFD[index][entry.GetTag()] = std::move(entry);
	}

	*offsetOfNextIFD = InterpretAs<uint32_t>(p);
	return CodecError::Success;
}

CodecError ExifReader::MMParseIFD(const uint8_t *imageFileHeader, size_t size, int index, uint32_t *offsetOfNextIFD)
{
	auto p = imageFileHeader + *offsetOfNextIFD;

	int numDirectoryEntries = MMInterpretAs<uint16_t>(p);
	p += sizeof(uint16_t);

	while (--numDirectoryEntries >= 0)
	{
		size_t offset = p - imageFileHeader;
		IFDEntry entry = { &bufferResource, imageFileHeader, offset, size, true };
		p += entry.size();
		IFD[index][entry.GetTag()] = std::move(entry);
	}

	*offsetOfNextIFD = MMInterpretAs<uint32_t>(p);
	return CodecError::Success;
}

CodecError ExifReader::ParseIFD0(const uint8_t *imageFileHeader, size_t size, const IFDEntry &entry)
{
	return CodecError::Success;
}

CodecError ExifReader::ParseIFD1(const uint8_t *imageFileHeader, size_t size, const IFDEntry &entry)
{
	return CodecError::Success;
}

CodecError ExifReader::ParseIFD2(const uint8_t *imageFileHeader, size_t size, const IFDEntry &entry)
{
	return CodecError::Success;
}

}
}
