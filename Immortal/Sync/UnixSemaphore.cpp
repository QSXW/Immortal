/**
 * Copyright (C) 2022, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#include "Sync/Semaphore.h"

namespace Immortal
{

Semaphore::Semaphore(const std::string &name, bool manaulReset) :
    handle{},
    reset{ manaulReset }
{

}

Semaphore::~Semaphore()
{

}

void Semaphore::Signal()
{

}

void Semaphore::Wait(uint32_t timeout)
{

}

void Semaphore::Reset()
{

}

}
