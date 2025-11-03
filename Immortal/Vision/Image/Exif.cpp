#include "Exif.h"
#include "Common/BitStream.h"

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
	tag{},
	type{ IFDType::Invalid },
    count{},
    offset{}
{

}

IFDEntry::IFDEntry(ByteStream &bs) :
    tag{bs.get_le<uint16_t>()},
    type{bs.get_le<uint16_t>()},
    count{bs.get_le<uint32_t>()},
    offset{bs.get_le<uint32_t>()},
    values{}
{

}

IFDEntry::IFDEntry(ByteStream &bs, bool be) :
    tag{bs.get_be<uint16_t>()},
    type{bs.get_be<uint16_t>()},
    count{bs.get_be<uint32_t>()},
    offset{bs.get_be<uint32_t>()},
    values{}
{

}

IFDEntry::~IFDEntry()
{

}

void IFDEntry::ReadValues(ByteStream &bs, bool be)
{
	bool valid = false;
	switch (type)
	{
		case IFDType::BYTE:
		case IFDType::SBYTE:
		case IFDType::ASCII:
			valid = ReadValue<uint8_t>(bs, be);
			if (type == IFDType::ASCII && values.back() != 0)
			{
				values.resize(values.size() + 1);
				values.back() = 0;
				v.ptr = values.data();
			}
			break;

		case IFDType::SHORT:
			valid = ReadValue<uint16_t>(bs, be);
			break;

		case IFDType::LONG:
			valid = ReadValue<uint32_t>(bs, be);
			break;

		case IFDType::RATIONAL:
			count <<= 1;
			valid = ReadValue<uint32_t>(bs, be);
			count >>= 1;
			break;

		case IFDType::UNDEFINED:
			valid = ReadValue<uint8_t>(bs, be);
			break;

		case IFDType::SSHORT:
			valid = ReadValue<int16_t>(bs, be);
			break;

		case IFDType::SLONG:
			valid = ReadValue<int32_t>(bs, be);
			break;

		case IFDType::SRATIONAL:
			count <<= 1;
			valid = ReadValue<int32_t>(bs, be);
			count >>= 1;
			break;

		case IFDType::FLOAT:
			valid = ReadValue<uint32_t>(bs, be);
			break;

		case IFDType::DOUBLE:
			valid = ReadValue<uint64_t>(bs, be);
			break;

		default:
			break;
	}

	if (!valid)
	{
		tag = {};
	}
}

ExifReader::ExifReader() :
    IFD{},
    flags{flags},
    be{}
{

}

ExifReader::ExifReader(const uint8_t *data, size_t size, ExifFlags flags) :
    ExifReader{}
{
	//for (int i = 0; i < SL_ARRAY_LENGTH(IFD); i++)
	//{
	//	IFD[i] = std::pmr::unordered_map<uint16_t, IFDEntry>{allocator};
	//}

	ByteStream bs{data, size};

	uint32_t offsetToFirstIDF = 0;
	if (ParseHeader(bs, offsetToFirstIDF, be) != CodecError::Success)
	{
		return;
	}

	if (ParseIFD(bs, IFD0, offsetToFirstIDF) != CodecError::Success)
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
				offsetToFirstIDF = it->second.GetValue<uint32_t>();
				if (ParseIFD(bs, SubIFD0 + i, offsetToFirstIDF) != CodecError::Success)
				{
					return;
				}
			}
		}
	}
}

CodecError ExifReader::ParseHeader(ByteStream &bs, uint32_t &offsetToFirstIDF, bool &be)
{
	auto endian = bs.get_le<uint16_t>();
	if (endian == 0x4D4D /* MM */)
	{
		be = true;
	}
	else if (endian != 0x4949 /* II */)
	{
		return CodecError::CorruptStream;
	}

	uint16_t _42 = 0;
	if (be)
	{
		_42 = bs.get_be<uint16_t>();
		offsetToFirstIDF = bs.get_be<uint32_t>();
	}
	else
	{
		_42 = bs.get_le<uint16_t>();
		offsetToFirstIDF = bs.get_le<uint32_t>();
	}

	if (_42 != 0x002a)
	{
		return CodecError::CorruptStream;
	}

	return CodecError::Success;
}

CodecError ExifReader::ParseIFD(ByteStream &bs, int startIFDindex, uint32_t offsetOfNextIFD)
{
	CodecError ret = {};
	for (int index = startIFDindex; offsetOfNextIFD != 0; index++)
	{
		bs.seek(offsetOfNextIFD, SEEK_SET);
		if (be)
		{
			if (MMParseIFD(bs, index, &offsetOfNextIFD) != CodecError::Success)
			{
				return ret;
			}
		}
		else
		{
			if (ParseIFD(bs, index, &offsetOfNextIFD) != CodecError::Success)
			{
				return ret;
			}
		}
	}

	return CodecError::Success;
}

enum
{
	EXIF_IFD             = 0x8769, // EXIF IFD
	GPS_IFD              = 0x8825, // GPS IFD
	INTEROPERABILITY_IFD = 0xA005,  // Interoperability IFD
	MAKERNOTE_TAG        = 0x927c
};

struct MakerHeader
{
	int offset;
	std::vector<uint8_t> header;
};

static const std::vector<MakerHeader> KMakers = {
	{  6, { 'A', 'O', 'C', 0, }},
	{ -1, { 'Q', 'V', 'C', 0, 0, 0, }},
	{ 10, { 'F', 'O', 'V', 'E', 'O', 'N', 0, 0, }},
	{ -1, { 'F', 'U', 'J', 'I', }},
	{  8, { 'O', 'L', 'Y', 'M', 'P', 0, }},
	{ -1, { 'O', 'L', 'Y', 'M', 'P', 'U', 'S', 0, 'I', 'I', }},
	{ 12, { 'P', 'a', 'n', 'a', 's', 'o', 'n', 'i', 'c', 0, 0, 0, }},
	{ 10, { 'S', 'I', 'G', 'M', 'A', 0, 0, 0, }},
	{ 12, { 'S', 'O', 'N', 'Y', ' ', 'D', 'S', 'C', ' ', 0, 0, 0, }},
};

#define EXIF_II 0x49492a00
#define EXIF_MM 0x4d4d002a

static int GetMakeNoteOffset(ByteStream &bs)
{
	if (bs.get_bytes_left() < 12)
	{
		return -1;
	}

	for (auto &maker : KMakers)
	{
		if (!memcmp(maker.header.data(), bs.buf, maker.header.size()))
		{
			return maker.offset;
		}
	}

	const uint8_t hNikon[] = { 'N', 'i', 'k', 'o', 'n', 0, };
    if (!memcmp(bs.buf, hNikon, sizeof(hNikon)))
	{
		if (bs.get_bytes_left() < 14)
		{
			return -1;
		}
		else if (InterpretAs<uint32_t>(bs.buf + 10) == EXIF_MM || InterpretAs<uint32_t>(bs.buf + 10) == EXIF_II)
		{
			return -1;
		}

		return 8;
	}

	return 0;
}

static int GetIndexByTag(int tag)
{
	switch (tag)
	{
		case EXIF_IFD:
			return IFD_EXIF;
		case GPS_IFD:
			return IFD_GPS;
		case INTEROPERABILITY_IFD:
			return IFD_INTEROP;
		case MAKERNOTE_TAG:
			return IFD_MAKENOTE;
		default:
			return -1;
	}
}

CodecError ExifReader::ParseIFD(ByteStream &bs, int index, uint32_t *offsetOfNextIFD, bool be)
{
	int n = be ? bs.get_be<uint16_t>() : bs.get_le<uint16_t>();
	while (--n >= 0)
	{
		IFDEntry entry{};
		if (be)
		{
			entry = {bs, be};
		}
		{
			entry = {bs};
		}

		auto tag = entry.tag;
		IFDType type = entry.type;
		if (type > IFDType::IFD)
		{
			return CodecError::CorruptedBitstream;
		}

		bool isIFD = type == IFDType::IFD || 
			         tag  == EXIF_IFD ||
			         tag  == GPS_IFD  ||
			         tag  == INTEROPERABILITY_IFD ||
		             tag  == MAKERNOTE_TAG;
		if (isIFD && !entry.offset)
		{
			return CodecError::CorruptedBitstream;
		}

		ByteStream _bs{bs.start, bs.size()};
		_bs.seek(entry.offset, SEEK_SET);
		if (tag == MAKERNOTE_TAG)
		{
			auto makeNoteOffset = GetMakeNoteOffset(_bs);
			if (!makeNoteOffset)
			{
				isIFD = false;
			}
			else
			{
				_bs.seek(makeNoteOffset, SEEK_CUR);
			}
		}

		if (isIFD)
		{
			entry.type = IFDType::IFD;
			if (!(flags & ExifFlags::DisabledRecursive))
			{
				ParseIFD(_bs, GetIndexByTag(tag), offsetOfNextIFD);
			}
		}
		else
		{
			entry.ReadValues(_bs, be);
		}

		IFD[index][entry.GetTag()] = std::move(entry);
	}

	*offsetOfNextIFD = be ? bs.get_be<uint32_t>() : bs.get_le<uint32_t>();
	return CodecError::Success;
}

CodecError ExifReader::MMParseIFD(ByteStream &bs, int index, uint32_t *offsetOfNextIFD)
{
	return ParseIFD(bs, index, offsetOfNextIFD, true);
}

}
}
