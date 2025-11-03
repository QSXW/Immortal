#pragma once

#include "Types.h"
#include "Picture.h"
#include "Codec.h"
#include "CodedFrame.h"

namespace Immortal
{
namespace Vision
{

Codec *SelectSuitableCodec(const std::string &path, bool decoder = false, const ImageEncodeInfo &info = {});

Picture Read(const String &path);

CodedFrame Write(const Picture &picture, const String &path = {}, const ImageEncodeInfo &info = {});

}
}
