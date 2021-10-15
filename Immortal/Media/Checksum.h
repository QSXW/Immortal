#pragma once

#include <stdint.h>

namespace Checksum
{

#define CRC32_GENERATOR_POLYNOMIAL 0x04C11DB7

uint32_t CyclicRedundancyCheck32(const uint8_t *message, uint32_t length);

}
