/**
 * Copyright (C) 2022, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#pragma once

#include "Core.h"

namespace Immortal
{

class Semaphore
{
public:
    Semaphore(const std::string &name = std::string{}, bool manaulReset = true);

    ~Semaphore();

    void Signal();

    void Wait(uint32_t timeout = std::numeric_limits<uint32_t>::max());

    void Reset();

    operator bool() const
    {
        return !!handle;
    }

protected:
    Anonymous handle;

    bool reset;
};

}
