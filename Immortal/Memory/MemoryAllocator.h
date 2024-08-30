/**
 * Copyright (C) 2022, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#pragma once

#ifndef MEMORY_ALLOCATOR_H_
#define MEMORY_ALLOCATOR_H_

#include "Core.h"
#include "Allocator.h"
#include "Shared/IObject.h"
#include <map>
#include <ctime>
#include <mutex>

// #define IMMORTRAL_ENABLE_MEMORY_PROFILE
#ifdef IMMORTRAL_ENABLE_MEMORY_PROFILE
#include <stacktrace>
#endif

#define FORWARD_NEW2MALLOC  void *operator new(size_t size) { return malloc(size); }
#define FORWARD_DELETE2FREE void operator delete(void *ptr) { free(ptr); }

namespace Immortal
{

#ifdef IMMORTRAL_ENABLE_MEMORY_PROFILE
using CStackTrace = std::basic_stacktrace<CAllocator<std::stacktrace_entry>>;
class StackTrace : public CStackTrace
{
public:
    StackTrace() :
        CStackTrace{}
    {

    }

	StackTrace(CStackTrace &&other) :
	    CStackTrace{std::move(other) }
    {

    }

	FORWARD_NEW2MALLOC
	FORWARD_DELETE2FREE
};
#endif

struct AllocationInfo
{
    const char *name;

    clock_t clock;

    size_t size;

#ifdef IMMORTRAL_ENABLE_MEMORY_PROFILE
    StackTrace stacktrace;
#endif
};

class Allocation : public std::map<uint64_t, AllocationInfo, std::less<uint64_t>, CAllocator<std::pair<const uint64_t, AllocationInfo>>>
{
public:
    Allocation() = default;

	FORWARD_NEW2MALLOC
	FORWARD_DELETE2FREE
};

class MemoryAllocator
{
public:
    static MemoryAllocator Instance;

public:
    MemoryAllocator();

    ~MemoryAllocator();

    void *Allocate(size_t size);

    void Free(Anonymous _ptr);

    void Release();

private:
    URef<Allocation> allocation;

    size_t allocatedSize = 0;

    std::mutex mutex;
};

}

#endif
