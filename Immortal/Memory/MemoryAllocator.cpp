/**
 * Copyright (C) 2022, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#include "MemoryAllocator.h"

#include <cstdio>

#ifdef IMMORTRAL_ENABLE_MEMORY_PROFILE
#ifdef _WIN32
#include <DbgHelp.h>
#ifdef _MSC_VER
#pragma comment(lib, "Dbghelp.lib")
#endif
#elif defined(__GNUC__) || defined(__clang__)
#include <execinfo.h>
#endif
#endif

namespace Immortal
{

namespace
{

thread_local bool memoryProfilerInternal = false;

class MemoryProfilerInternalScope
{
public:
    MemoryProfilerInternalScope() :
        previous{ memoryProfilerInternal }
    {
        memoryProfilerInternal = true;
    }

    ~MemoryProfilerInternalScope()
    {
        memoryProfilerInternal = previous;
    }

private:
    bool previous;
};

#ifdef IMMORTRAL_ENABLE_MEMORY_PROFILE
StackTrace CaptureAllocationStack() noexcept
{
    StackTrace stackTrace{};

#ifdef _WIN32
    stackTrace.depth = static_cast<uint16_t>(CaptureStackBackTrace(
        4,
        static_cast<DWORD>(stackTrace.frames.size()),
        reinterpret_cast<void **>(stackTrace.frames.data()),
        nullptr));
#elif defined(__GNUC__) || defined(__clang__)
    std::array<void *, MemoryProfileStackDepth + 4> frames{};
    const int captured = backtrace(frames.data(), static_cast<int>(frames.size()));
    if (captured > 4)
    {
        stackTrace.depth = static_cast<uint16_t>(std::min<int>(captured - 4, static_cast<int>(MemoryProfileStackDepth)));
        for (uint16_t i = 0; i < stackTrace.depth; ++i)
        {
            stackTrace.frames[i] = reinterpret_cast<uintptr_t>(frames[i + 4]);
        }
    }
#endif

    return stackTrace;
}

#ifdef _WIN32
void PrintStackTrace(HANDLE process, const StackTrace &stackTrace)
{
    for (uint16_t i = 0; i < stackTrace.depth; ++i)
    {
        const DWORD64 address = static_cast<DWORD64>(stackTrace.frames[i]);
        alignas(SYMBOL_INFO) char symbolStorage[sizeof(SYMBOL_INFO) + MAX_SYM_NAME]{};
        SYMBOL_INFO *symbol = reinterpret_cast<SYMBOL_INFO *>(symbolStorage);
        symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
        symbol->MaxNameLen = MAX_SYM_NAME;

        DWORD64 symbolDisplacement = 0;
        const bool hasSymbol = SymFromAddr(process, address, &symbolDisplacement, symbol) == TRUE;

        IMAGEHLP_LINE64 line{};
        line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
        DWORD lineDisplacement = 0;
        const bool hasLine = SymGetLineFromAddr64(process, address, &lineDisplacement, &line) == TRUE;

        if (hasLine && hasSymbol)
        {
            printf("    %s(%lu): %s+0x%llX\n", line.FileName, line.LineNumber, symbol->Name, symbolDisplacement);
        }
        else if (hasSymbol)
        {
            printf("    0x%llX: %s+0x%llX\n", address, symbol->Name, symbolDisplacement);
        }
        else
        {
            printf("    0x%llX\n", address);
        }
    }
}
#else
void PrintStackTrace(const StackTrace &stackTrace)
{
    for (uint16_t i = 0; i < stackTrace.depth; ++i)
    {
        printf("    0x%zx\n", static_cast<size_t>(stackTrace.frames[i]));
    }
}
#endif
#endif

}

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4073)
#pragma init_seg(lib)
#pragma warning(pop)
#endif
MemoryAllocator MemoryAllocator::Instance;

MemoryAllocator::MemoryAllocator()
{
    MemoryProfilerInternalScope scope;
    allocation = new Allocation;
    allocation->reserve(65536);

#ifdef IMMORTRAL_ENABLE_MEMORY_PROFILE
    stackTraces = new StackTraceRecords;
    stackTraces->reserve(4096);
    workerRunning.store(true, std::memory_order_release);
    worker = std::thread{ [this] {
        memoryProfilerInternal = true;
        WorkerLoop();
    } };
#endif
}

MemoryAllocator::~MemoryAllocator()
{
    Release();
}

void *MemoryAllocator::Allocate(size_t size)
{
    Anonymous address = malloc(size);
    if (!address)
    {
        return nullptr;
    }

    if (!allocation || memoryProfilerInternal)
    {
        return address;
    }

#ifdef IMMORTRAL_ENABLE_MEMORY_PROFILE
    {
        MemoryProfilerInternalScope scope;
        AllocationEvent event{};
        event.type = AllocationEventType::Allocate;
        event.address = reinterpret_cast<uintptr_t>(address);
        event.size = size;
        event.stackTrace = CaptureAllocationStack();
        Enqueue(event);
    }
#else
    {
        std::lock_guard lock{ mutex };
        allocation->insert({ reinterpret_cast<uintptr_t>(address), AllocationInfo{ size } });
    }
    allocatedSize.fetch_add(size, std::memory_order_relaxed);
#endif

    return address;
}

void MemoryAllocator::Free(Anonymous _ptr)
{
    if (!_ptr)
    {
        return;
    }

    if (!allocation || memoryProfilerInternal)
    {
        free(_ptr);
        return;
    }

#ifdef IMMORTRAL_ENABLE_MEMORY_PROFILE
    {
        MemoryProfilerInternalScope scope;
        AllocationEvent event{};
        event.type = AllocationEventType::Free;
        event.address = reinterpret_cast<uintptr_t>(_ptr);
        Enqueue(event);
    }
#else
    {
        const uintptr_t ptr = reinterpret_cast<uintptr_t>(_ptr);
        std::lock_guard lock{ mutex };
        if (allocation->find(ptr) != allocation->end())
        {
            allocation->erase(ptr);
        }
    }
#endif
    free(_ptr);
}

#ifdef IMMORTRAL_ENABLE_MEMORY_PROFILE
void MemoryAllocator::Enqueue(const AllocationEvent &event) noexcept
{
    events.Write(event);
    queueVersion.fetch_add(1, std::memory_order_release);
    queueVersion.notify_one();
}

void MemoryAllocator::Process(const AllocationEvent &event)
{
    if (event.type == AllocationEventType::Allocate)
    {
        allocatedSize.fetch_add(event.size, std::memory_order_relaxed);
        auto [trace, inserted] = stackTraces->try_emplace(event.stackTrace, StackTraceStats{});
        (void)inserted;
        StackTraceStats &stats = trace->second;
        ++stats.allocationCount;
        ++stats.liveAllocationCount;
        stats.liveSize += event.size;

        const auto existing = allocation->find(event.address);
        if (existing != allocation->end())
        {
            StackTraceStats *oldStats = existing->second.stackTrace;
            if (oldStats->liveAllocationCount > 0)
            {
                --oldStats->liveAllocationCount;
            }
            oldStats->liveSize = oldStats->liveSize >= existing->second.size
                ? oldStats->liveSize - existing->second.size
                : 0;
            allocation->erase(existing);
        }

        allocation->insert({ event.address, AllocationInfo{ event.size, &stats } });
        return;
    }

    const auto allocationEntry = allocation->find(event.address);
    if (allocationEntry == allocation->end())
    {
        return;
    }

    StackTraceStats *stats = allocationEntry->second.stackTrace;
    if (stats->liveAllocationCount > 0)
    {
        --stats->liveAllocationCount;
    }
    stats->liveSize = stats->liveSize >= allocationEntry->second.size
        ? stats->liveSize - allocationEntry->second.size
        : 0;
    allocation->erase(allocationEntry);
}

void MemoryAllocator::WorkerLoop()
{
    for (;;)
    {
        AllocationEvent event{};
        while (events.Read(event))
        {
            Process(event);
        }

        if (!workerRunning.load(std::memory_order_acquire))
        {
            while (events.Read(event))
            {
                Process(event);
            }
            break;
        }

        const uint64_t version = queueVersion.load(std::memory_order_acquire);
        if (events.Read(event))
        {
            Process(event);
            continue;
        }
        queueVersion.wait(version, std::memory_order_acquire);
    }
}

void MemoryAllocator::StopWorker()
{
    workerRunning.store(false, std::memory_order_release);
    queueVersion.fetch_add(1, std::memory_order_release);
    queueVersion.notify_all();

    if (worker.joinable())
    {
        MemoryProfilerInternalScope scope;
        worker.join();
    }
}
#endif

void MemoryAllocator::Release()
{
    if (!allocation)
    {
        return;
    }

#ifdef IMMORTRAL_ENABLE_MEMORY_PROFILE
    StopWorker();
#endif

    MemoryProfilerInternalScope scope;
    const size_t totalAllocatedSize = allocatedSize.load(std::memory_order_relaxed);
    printf("Total Size Allocated: %zu (Bytes), %g (Mb)\n", totalAllocatedSize, totalAllocatedSize / 1048576.0);
    size_t leakSize = 0;
    size_t leakCount = 0;
    size_t groupCount = 0;

#ifdef IMMORTRAL_ENABLE_MEMORY_PROFILE
#ifdef _WIN32
    HANDLE process = GetCurrentProcess();
    SymSetOptions(SymGetOptions() | SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES | SYMOPT_UNDNAME);
    const bool ownsSymbolHandler = SymInitialize(process, nullptr, TRUE) == TRUE;
#endif

    for (const auto &[stackTrace, stats] : *stackTraces)
    {
        if (stats.liveAllocationCount == 0)
        {
            continue;
        }

        leakSize += stats.liveSize;
        leakCount += stats.liveAllocationCount;
        printf(
            "[%zu] %zu allocation(s), %zu byte(s) leaked at >>>\n",
            groupCount,
            stats.liveAllocationCount,
            stats.liveSize);
#ifdef _WIN32
        PrintStackTrace(process, stackTrace);
#else
        PrintStackTrace(stackTrace);
#endif
        printf("<<<\n");
        ++groupCount;
    }

#ifdef _WIN32
    if (ownsSymbolHandler)
    {
        SymCleanup(process);
    }
#endif
#else
    leakCount = allocation->size();
    for (const auto &[address, info] : *allocation)
    {
        (void)address;
        leakSize += info.size;
    }
#endif

    printf("Total Allocations Leaked: %zu in %zu call stack(s)\n", leakCount, groupCount);
    printf("Total Size Leaked: %zu\n", leakSize);

    allocation = nullptr;
#ifdef IMMORTRAL_ENABLE_MEMORY_PROFILE
    stackTraces = nullptr;
#endif
}

}

extern "C"
{

void *iml_allocate(size_t size)
{
    auto &allocator = Immortal::MemoryAllocator::Instance;
    return allocator.Allocate(size);
}

void iml_release(void *ptr) noexcept
{
    auto &allocator = Immortal::MemoryAllocator::Instance;
    allocator.Free(ptr);
}

}
