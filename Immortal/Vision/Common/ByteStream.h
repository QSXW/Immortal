#pragma once

#include <cstdint>
#include <utility>

namespace Immortal
{
namespace Vision
{

#define BYTESWAP16C(x) (((x) << 8 & 0xff00) | ((x) >> 8 & 0x00ff))
#define BYTESWAP32C(x) (BYTESWAP16C(x) << 16 | BYTESWAP16C((x) >> 16))
#define BYTESWAP64C(x) (BYTESWAP32C(x) << 32 | BYTESWAP32C((x) >> 32))

class ByteStream
{
public:
	ByteStream(const uint8_t *buf, size_t size) :
	    buf{buf},
	    start{buf},
	    end{buf + size},
	    eof{0}
	{

	}

    template <class T>
    T get_le()
    {
        buf += sizeof(T); return (*((const __unaligned T*)(buf - sizeof(T))));
    }

    template <class T>
    T get_be()
    {
        if constexpr (sizeof(T) == 2)
        {
			return (T)BYTESWAP16C(get_le<T>());
        }
		else if constexpr (sizeof(T) == 4)
		{
			return (T)BYTESWAP32C(get_le<T>());
		}
		else if constexpr(sizeof(T) == 8)
		{
			return (T)BYTESWAP32C(get_le<T>());
		}

        return (T)get_le<T>();
    }

    uint8_t get_byte()
    {
		return get_le<uint8_t>();
    }

    uint32_t get_le24()
    {
        uint32_t value = 0;
        return (get_le<uint8_t>() << 16) | (get_le<uint8_t>() << 8) | get_le<uint8_t>();
    }

    uint32_t get_be24()
    {
		buf += 3;
		return (((const uint8_t*)(*buf - 3))[0] << 16);
    }

    size_t get_buffer(uint8_t *dst, size_t size)
    {
		memcpy(dst, buf, size);
		buf += size;
		return size;
    }

    int get_bytes_left()
    {
        return end - buf;
    }

    int get_bytes_left_p()
    {
        return end - buf;
    }

    void skip(unsigned int size)
    {
        buf += std::min<size_t>(end - buf, size);
    }

    void skipu(unsigned int size)
    {
        buf += size;
    }

    void skip_p(unsigned int size)
    {
        unsigned int size2;
        if (eof)
        {
            return;
        }
		size2 = std::min<size_t>(end - buf, size);
        if (size2 != size)
            eof = 1;
        buf += size2;
    }

    int tell()
    {
        return (int)(buf - start);
    }

    int tell_p()
    {
        return (int)(buf - start);
    }

    size_t size()
    {
		return (size_t) (end - start);
    }

    int size_p()
    {
        return (int)(end - start);
    }

    int seek(int offset, int whence)
    {
        switch (whence) {
        case SEEK_CUR:
            offset = std::clamp<int64_t>(offset, -(buf - start),
                                end - buf);
            buf += offset;
            break;
        case SEEK_END:
			offset = std::clamp<int64_t>(offset, -(end - start), 0);
            buf = end + offset;
            break;
        case SEEK_SET:
			offset = std::clamp<int64_t>(offset, 0, end - start);
            buf = start + offset;
            break;
        default:
            return -1;
        }
        return tell();
    }

    int seek_p(int offset, int whence)
    {
        eof = 0;
        switch (whence) {
        case SEEK_CUR:
            if (end - buf < offset)
                eof = 1;
			offset = std::clamp<int64_t>(offset, -(buf - start),
                                end - buf);
            buf += offset;
            break;
        case SEEK_END:
            if (offset > 0)
                eof = 1;
			offset = std::clamp<int64_t>(offset, -(end - start), 0);
            buf = end + offset;
            break;
        case SEEK_SET:
            if (end - start < offset)
                eof = 1;
			offset = std::clamp<int64_t>(offset, 0, end - start);
            buf = start + offset;
            break;
        default:
            return -1;
        }
        return tell_p();
    }

public:
    const uint8_t *buf;
    const uint8_t *start;
    const uint8_t *end;
    int eof;
};

}
}
