#pragma once

#ifndef BIT_TRACKER_H__
#define BIT_TRACKER_H__

#include "Core.h"
#include "Media/LookupTable/LookupTable.h"

namespace Immortal
{

class IMMORTAL_API BitTracker
{
public:
    static constexpr size_t BytesMoved  = 8;
    static constexpr size_t BitsPerByte = 8;

public:
    BitTracker() :
        start{ nullptr },
        end{ nullptr },
        ptr{ nullptr },
        bitsLeft{ 0 },
        word{ 0 }
    {

    }

    BitTracker(const uint8_t *data, size_t size) :
        start{ data },
        end{ data + size },
        ptr{ data },
        bitsLeft{ 0 },
        word{ 0 }
    {
        Move(64);
    }

    template <size_t bytes>
    uint64_t GetBytes()
    {
        static_assert(bytes > 4 && "Unsupported bytes number");
        return GetBits<>(bytes * BitsPerByte);
    }

    uint64_t GetBits(uint32_t n)
    {
        if (n > bitsLeft)
        {
            Move(n);
        }

        uint64_t ret = word >> (64 - n);
        bitsLeft -= n;
        word <<= n;

        return ret;
    }

    uint64_t GetBit()
    {
        if (!bitsLeft)
        {
            Move(64);
        }

        uint64_t ret = word >> (64 - 1);
        bitsLeft--;
        word <<= 1;

        return ret;
    }

    void SkipBits(int32_t n)
    {
        if (n > bitsLeft)
        {
            n -= bitsLeft;
            word = bitsLeft = 0;
            Move(64);
        }
        bitsLeft -= n;
        word <<= n;
    }

    uint64_t Preview(uint32_t n)
    {
        if (n > bitsLeft)
        {
            Move(n);
        }
        return word >> (64 - n);
    }

    uint64_t UnsignedExpGolomb()
    {
        auto bits = Preview(32);
        auto l = 31 - (int32_t)std::log2(bits);
        SkipBits(l);

        return GetBits(l + 1) - 1;
    }

    uint64_t SignedExpGolomb()
    {
        auto bits = Preview(32);
        auto s = 31 - (int32_t)std::log2(bits);
        SkipBits(s);

        return LookupTable::ExponentialGolobm[s][GetBits(s + 1) - 1];
    }

    bool ByteAligned()
    {
        return !(bitsLeft & 0x7);
    }

    bool NoMoreData() const
    {
#ifdef _DEBUG
        return eof == EOF;
#else
        return !ptr;
#endif
    }

protected:
    void Move(uint32_t n)
    {
        uint64_t bits = 0;
        while (n > bitsLeft)
        {
            if (ptr < end)
            {
                bits <<= 8;
                bitsLeft += BitsPerByte;
                bits |= *ptr++;
            }
            else
            {
#ifdef _DEBUG
                eof = EOF;
#else
                ptr = nullptr;
#endif
                break;
            }
        }
        word |= bits << (64 - bitsLeft);
    }

protected:
    const uint8_t *start;
    const uint8_t *end;
    const uint8_t *ptr;
    uint64_t word;
    uint8_t bitsLeft;
#ifdef _DEBUG
    int32_t eof = 0;
#endif

};

using SuperBitTracker = BitTracker;

}

#endif
