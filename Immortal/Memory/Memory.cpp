/**
 * Copyright (C) 2022, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#include "Memory.h"

#ifdef _DEBUG

extern "C"
{

void *iml_allocate(size_t size);

void iml_release(void *ptr) noexcept;

}

void *operator new(size_t size)
{
    return iml_allocate(size);
}

void operator delete(void *ptr) noexcept
{
    iml_release(ptr);
}
#endif
