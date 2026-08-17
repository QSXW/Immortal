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
#include <array>
#include <atomic>
#include <mutex>
#include <thread>
#include <type_traits>
#include <unordered_map>

#define IMMORTRAL_ENABLE_MEMORY_PROFILE

#define FORWARD_NEW2MALLOC  void *operator new(size_t size) { return malloc(size); }
#define FORWARD_DELETE2FREE void operator delete(void *ptr) { free(ptr); }

namespace Immortal
{

static constexpr size_t MemoryProfileStackDepth = 32;

struct StackTrace
{
    uint16_t depth = 0;

    std::array<uintptr_t, MemoryProfileStackDepth> frames{};

    bool operator==(const StackTrace &other) const noexcept
    {
        if (depth != other.depth)
        {
            return false;
        }

        for (uint16_t i = 0; i < depth; ++i)
        {
            if (frames[i] != other.frames[i])
            {
                return false;
            }
        }
        return true;
    }
};

struct StackTraceHash
{
    size_t operator()(const StackTrace &stackTrace) const noexcept
    {
        size_t hash = static_cast<size_t>(stackTrace.depth);
        for (uint16_t i = 0; i < stackTrace.depth; ++i)
        {
            hash ^= static_cast<size_t>(stackTrace.frames[i]) +
                static_cast<size_t>(0x9e3779b97f4a7c15ull) +
                (hash << 6) +
                (hash >> 2);
        }
        return hash;
    }
};

struct StackTraceStats
{
    size_t allocationCount = 0;

    size_t liveAllocationCount = 0;

    size_t liveSize = 0;
};

struct AllocationInfo
{
    size_t size;

#ifdef IMMORTRAL_ENABLE_MEMORY_PROFILE
    StackTraceStats *stackTrace;
#endif
};

class Allocation : public std::unordered_map<
    uintptr_t,
    AllocationInfo,
    std::hash<uintptr_t>,
    std::equal_to<uintptr_t>,
    CAllocator<std::pair<const uintptr_t, AllocationInfo>>>
{
public:
    Allocation() = default;

	FORWARD_NEW2MALLOC
	FORWARD_DELETE2FREE
};

#ifdef IMMORTRAL_ENABLE_MEMORY_PROFILE
class StackTraceRecords : public std::unordered_map<
    StackTrace,
    StackTraceStats,
    StackTraceHash,
    std::equal_to<StackTrace>,
    CAllocator<std::pair<const StackTrace, StackTraceStats>>>
{
public:
    StackTraceRecords() = default;

    FORWARD_NEW2MALLOC
    FORWARD_DELETE2FREE
};

enum class AllocationEventType : uint8_t
{
    Allocate,
    Free
};

struct AllocationEvent
{
    AllocationEventType type = AllocationEventType::Allocate;

    uintptr_t address = 0;

    size_t size = 0;

    StackTrace stackTrace;
};

template <class T, size_t Capacity>
class ReadWriteQueue
{
    static_assert(Capacity > 1 && (Capacity & (Capacity - 1)) == 0);
    static_assert(std::is_trivially_copyable_v<T>);

public:
    void Write(const T &value) noexcept
    {
        // A single serialized write order keeps cross-thread allocate/free
        // events ordered while the reader remains independent.
        while (writeLock.test_and_set(std::memory_order_acquire))
        {
#ifdef _WIN32
            YieldProcessor();
#else
            std::this_thread::yield();
#endif
        }

        const size_t write = writeIndex.load(std::memory_order_relaxed);
        while (write - readIndex.load(std::memory_order_acquire) >= Capacity)
        {
            std::this_thread::yield();
        }

        entries[write & (Capacity - 1)] = value;
        writeIndex.store(write + 1, std::memory_order_release);
        writeLock.clear(std::memory_order_release);
    }

    bool Read(T &value) noexcept
    {
        const size_t read = readIndex.load(std::memory_order_relaxed);
        if (read == writeIndex.load(std::memory_order_acquire))
        {
            return false;
        }

        value = entries[read & (Capacity - 1)];
        readIndex.store(read + 1, std::memory_order_release);
        return true;
    }

private:
    std::array<T, Capacity> entries{};

    alignas(64) std::atomic_size_t readIndex = 0;

    alignas(64) std::atomic_size_t writeIndex = 0;

    alignas(64) std::atomic_flag writeLock = ATOMIC_FLAG_INIT;
};
#endif

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
#ifdef IMMORTRAL_ENABLE_MEMORY_PROFILE
    void Enqueue(const AllocationEvent &event) noexcept;

    void Process(const AllocationEvent &event);

    void WorkerLoop();

    void StopWorker();
#endif

private:
    URef<Allocation> allocation;

#ifdef IMMORTRAL_ENABLE_MEMORY_PROFILE
    URef<StackTraceRecords> stackTraces;

    ReadWriteQueue<AllocationEvent, 8192> events;

    std::atomic_bool workerRunning = false;

    std::atomic_uint64_t queueVersion = 0;

    std::thread worker;
#else
    std::mutex mutex;
#endif

    std::atomic_size_t allocatedSize = 0;
};

}

#endif
