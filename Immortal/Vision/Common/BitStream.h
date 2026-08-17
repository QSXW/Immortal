#pragma once

#include <cassert>
#include <cstdint>
#include <intrin.h>

namespace Immortal
{
namespace Vision
{

#define BSWAP16C(x) (((x) << 8 & 0xff00) | ((x) >> 8 & 0x00ff))
#define BSWAP32C(x) (BSWAP16C(x) << 16 | BSWAP16C((x) >> 16))
#define BSWAP64C(x) (BSWAP32C(x) << 32 | BSWAP32C((x) >> 32))

static inline int clz(const unsigned int mask)
{
	unsigned long leading_zero = 0;
	_BitScanReverse(&leading_zero, mask);
	return (31 - leading_zero);
}

static inline int ulog2(const unsigned v)
{
	return 31 ^ clz(v);
}

static inline unsigned inv_recenter(const unsigned r, const unsigned v)
{
	if (v > (r << 1))
		return v;
	else if ((v & 1) == 0)
		return (v >> 1) + r;
	else
		return r - ((v + 1) >> 1);
}

class BitStream
{
public:
	uint64_t state;
	int bits_left, error;
	const uint8_t *ptr, *ptr_start, *ptr_end;

	BitStream(const uint8_t *const data = nullptr, const size_t sz = 0) :
        ptr_start{data},
        ptr{ptr_start},
        ptr_end{ptr_start + sz},
        state{},
        bits_left{},
        error{}
    {

    }

	unsigned get_bit()
    {
        if (!bits_left)
        {
            if (ptr >= ptr_end)
            {
                error = 1;
            }
            else
            {
                const unsigned state = *ptr++;
                bits_left = 7;
                this->state = (uint64_t)state << 57;
                return state >> 7;
            }
        }

        const uint64_t state = this->state;
        bits_left--;
        this->state = state << 1;
        return (unsigned) (state >> 63);
    }

	inline void refill(const int n)
	{
		assert(bits_left >= 0 && bits_left < 32);
		unsigned state = 0;
		do
		{
			if (ptr >= ptr_end)
			{
				error = 1;
				if (state)
					break;
				return;
			}
			state = (state << 8) | *ptr++;
			bits_left += 8;
		} while (n > bits_left);
		state |= (uint64_t) state << (64 - bits_left);
	}

    template <class T, class U>
	T get_bits_t(const int n)
	{
		assert(n > 0 && n <= 32);
		/* Unsigned cast avoids refill after eob */
		if ((unsigned) n > (unsigned) bits_left)
			refill(n);
		const uint64_t state = this->state;
		bits_left -= n;
		this->state = state << n;
		return (T) ((U) state >> (64 - n));
	}

    unsigned get_bits(const int n)
    {
        return get_bits_t<unsigned, uint64_t>(n);
    }

    int get_sbits(const int n)
    {
        return get_bits_t<int, int64_t>(n);
    }

	unsigned get_uleb128()
	{
		uint64_t val = 0;
		unsigned i = 0, more;

		do
		{
			const int v = get_bits(8);
			more = v & 0x80;
			val |= ((uint64_t) (v & 0x7F)) << i;
			i += 7;
		} while (more && i < 56);

		if (val > UINT32_MAX || more)
		{
			this->error = 1;
			return 0;
		}

		return (unsigned) val;
	}

	unsigned get_uniform(const unsigned max)
	{
		// Output in range [0..max-1]
		// max must be > 1, or else nothing is read from the bitstream
		assert(max > 1);
		const int l = ulog2(max) + 1;
		assert(l > 1);
		const unsigned m = (1U << l) - max;
		const unsigned v = get_bits(l - 1);
		return v < m ? v : (v << 1) - m + get_bit();
	}

	unsigned get_vlc()
	{
		if (get_bit())
			return 0;

		int n_bits = 0;
		do
		{
			if (++n_bits == 32)
				return UINT32_MAX;
		} while (!get_bit());

		return ((1U << n_bits) - 1) + get_bits(n_bits);
	}

	unsigned get_bits_subexp_u(const unsigned ref,
	                                  const unsigned n)
	{
		unsigned v = 0;

		for (int i = 0;; i++)
		{
			const int b = i ? 3 + i - 1 : 3;

			if (n < v + 3 * (1 << b))
			{
				v += get_uniform(n - v + 1);
				break;
			}

			if (!get_bit())
			{
				v += get_bits(b);
				break;
			}

			v += 1 << b;
		}

		return ref * 2 <= n ? inv_recenter(ref, v) : n - inv_recenter(n - ref, v);
	}

	int get_bits_subexp(const int ref, const unsigned n)
	{
		return (int) get_bits_subexp_u(ref + (1 << n), 2 << n) - (1 << n);
	}

	uint32_t get_le32()
	{
		uint32_t v = get_bits(32);
		return BSWAP32C(v);
	}

	uint32_t get_le24()
	{
		return (get_bits(8) << 16) | (get_bits(8) << 8) | get_bits(8);
	}

	uint32_t get_le16()
	{
		uint32_t v = get_bits(16);
		return BSWAP16C(v);
	}
};

}
}
