#pragma once

#include "Core.h"

namespace Immortal
{

union Colour
{
    struct { float r, g, b, a; };
    float       f4[4];
    int32_t     int32[4];
    uint32_t    uint32[4];

    Colour() :
        f4{ 0.0f, 0.0f, 0.0f, 1.0f }
    {
      
    }

    Colour(float r, float g, float b, float a) :
        f4{ r, g, b, a }
    {

    }
};

struct DepthStencilValue
{
    float depth;
    uint32_t stencil;
};

}
