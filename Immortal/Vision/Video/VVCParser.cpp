#include "VVCParser.h"
#include "Common/BitTracker.h"

namespace Immortal
{
namespace Vision
{


struct NALHeader
{

};

CodecError VVCParser::Parse(const uint8_t *buffer, size_t size)
{
	BitStream bs = { buffer ,size };

	auto m_forbiddenZeroBit   = bs.GetBits(1);
	auto m_nuhReservedZeroBit = bs.GetBits(1);
	auto m_nuhLayerId         = bs.GetBits(6);
	auto m_nalUnitType        = bs.GetBits(5);
	auto m_temporalId         = bs.GetBits(3) - 1;         
	return CodecError::Succeed;
}

}
}
