#pragma once

#ifndef BIT_TRACKER_H__
#define BIT_TRACKER_H__

#include "Core.h"

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

    uint64_t ue()
    {
        uint64_t ret = 0;
        int leadingZeroBits = -1;

        for (int bit = 0; !bit; leadingZeroBits++)
        {
            bit = GetBits(1);
        }
        ret = std::pow(2, leadingZeroBits) - 1 + GetBits(leadingZeroBits);

        return ret;
    }

    bool ByteAligned()
    {
        return bitsLeft == 0;
    }

protected:
    void Move(uint32_t n)
    {
        uint64_t bits = 0;
        while (n > bitsLeft)
        {
            bits <<= 8;
            bitsLeft += BitsPerByte;
            if (ptr < end)
            {
                bits |= *ptr++;
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
};

using SuperBitTracker = BitTracker;

}

#endif
