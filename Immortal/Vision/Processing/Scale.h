/**
 * Copyright (C) 2024, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#include "Core.h"
#include "Picture.h"

namespace Immortal
{
namespace Vision
{

void BicubicConvolutionInterpolate(Picture &dst, const Picture &src);

void ScaleTo8Bits(Picture &dst, const Picture &src);

}
}
