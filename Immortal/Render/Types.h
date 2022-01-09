#pragma once

#include "Core.h"

namespace Immortal
{

union Colour
{
    struct { float r, g, b, a; };
    Vector4     v4;
    float       float32[4];
    int32_t     int32[4];
    uint32_t    uint32[4];

    Colour() :
        v4{ 0.0f, 0.0f, 0.0f, 1.0f }
    {
      
    }

    Colour(Vector4 v) :
        v4{ v }
    {
    
    }

    Colour(float r, float g, float b, float a) :
        float32{ r, g, b, a }
    {

    }
};

struct DepthStencilValue
{
    float depth;
    uint32_t stencil;
};

}
