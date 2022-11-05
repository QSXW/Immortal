/**
 * Copyright (C) 2022, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#include "Sync/Semaphore.h"

#include <synchapi.h>
#include <handleapi.h>

namespace Immortal
{

Semaphore::Semaphore(const std::string &name, bool manaulReset) :
    handle{},
    reset{ manaulReset }
{
    handle = CreateEventA(NULL, manaulReset, FALSE, name.empty() ? NULL : name.c_str());
}

Semaphore::~Semaphore()
{
    if (handle)
    {
        CloseHandle(handle);
        handle = nullptr;
    }
}

void Semaphore::Signal()
{
    SetEvent(handle);
}

void Semaphore::Wait(uint32_t timeout)
{
    DWORD ret = WaitForSingleObjectEx(handle, timeout, FALSE);
    if (ret != WAIT_OBJECT_0)
    {
        LOG::ERR("Semaphore wait error `{}`", ret);
    }
    if (reset)
    {
        ResetEvent(handle);
    }
}

void Semaphore::Reset()
{
    ResetEvent(handle);
}

}
