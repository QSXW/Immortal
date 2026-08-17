#include "TIFF.h"
#include "Vision/Video/FFCodec.h"

namespace Immortal
{
namespace Vision
{

TIFFCodec::TIFFCodec() :
    Super{}
{
}

TIFFCodec::~TIFFCodec()
{

}

CodecError TIFFCodec::Decode(const CodedFrame &codedFrame)
{
	return CodecError::NotImplement;
}

CodecError TIFFCodec::Encode(const Picture &picture, CodedFrame &codedFrame)
{
	return CodecError::NotImplement;
}

}
}
