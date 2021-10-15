#include "Checksum.h"

namespace Checksum
{

uint32_t CyclicRedundancyCheck32(const uint8_t *message, uint32_t length)
{
    uint32_t CRC = (uint32_t)~0;

    for (int i = 0; i < length; i++)
    {
        CRC ^= (((uint32_t)message[i]) << 24);
        for (int j = 0; j < 8; j++)
        {
            uint32_t MSB = CRC >> 31;
            CRC <<= 1;
            CRC ^= (0 - MSB) & CRC32_GENERATOR_POLYNOMIAL;
        }
    }
    
    return CRC;
}

}
