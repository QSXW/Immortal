#pragma once

#include "Core.h"

namespace Immortal
{

class Animator
{
public:
    Animator()
    {
        memset(this, 0, sizeof(*this));
    }

public:
    struct {
        int64_t last;
        int64_t current;
    } timestamps;

    double timebase;
    double spf;
    uint32_t step;
    uint32_t frameNumber;
};

}
