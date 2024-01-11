#include "RawSpeed.h"
#include "rawspeed_capi.h"

namespace Immortal
{ 
namespace Vision
{

RawSpeedCodec::RawSpeedCodec()
{

}

CodecError RawSpeedCodec::Decode(const CodedFrame &codedFrame)
{
    const auto &buffer = codedFrame.buffer;

    auto parser = rawspeed_parser_allocate(buffer.data(), buffer.size());

    auto decoder = rawspeed_parser_get_decoder(parser);
    auto image = rawspeed_decoder_decode_raw(decoder);

    RawImageParams params{};
	rawspeed_image_get_params(image, &params);
	auto width  = params.width;
	auto height = params.height;
	picture = Picture{ width, height, Format::BayerLayerRGBG, true};
    
    rawspeed_image_fill_data(image, picture.GetData(), width * picture.GetFormat().GetTexelSize());

    rawspeed_image_release(&image);
    rawspeed_parser_release(&parser);

    return CodecError::Succeed;
}

}
}
