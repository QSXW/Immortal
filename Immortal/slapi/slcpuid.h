#pragma once

#include <cstdint>

#ifdef _MSC_VER
#include <intrin.h>
#endif

enum class CPUFlag : uint32_t
{
    None      = 0,
    FPU       = 1 <<  0,
    MMX       = 1 <<  1,
    SSE       = 1 <<  2,
    SSE2      = 1 <<  3,
    SSE3      = 1 <<  4,
    SSSE3     = 1 <<  5,
    SSE4      = 1 <<  6,
    PCLMULQDQ = 1 <<  7,
    AVX       = 1 <<  8,
    AVX2      = 1 <<  9,
    AVX512    = 1 << 10,
    F16C      = 1 << 11,
    NEON      = 1 << 12,
};

template <class T>
requires std::is_same_v<T, CPUFlag> void operator|=(T &a, const T b)
{
    a = (T)((uint32_t)a | (uint32_t)b);
}

template <class T>
requires std::is_same_v<T, CPUFlag> T operator&(T a, T b)
{
    return (T)((uint32_t)a & (uint32_t)b);
}

class CPU
{
private:
    static inline CPUFlag cpu_flags = CPUFlag::None;

    static inline bool enabled = false;

    struct DataRegisters
    {
        uint32_t eax;
        uint32_t ebx;
        uint32_t ecx;
        uint32_t edx;
    };

    static void invoke_cpuid(DataRegisters *registers, int function_id)
    {
#ifdef _MSC_VER
        __cpuid((int *)registers, function_id);
#endif
    }

    static void invoke_cpuidex(DataRegisters *registers, int function_id,int subfunction_id)
    {
#ifdef _MSC_VER
        __cpuidex((int *)registers, function_id, subfunction_id);
#endif
    }

    static void extract_info1(uint32_t ebx)
    {
        if (IsSupported(CPUFlag::AVX) && (ebx & (1 << 5)))
        {
            cpu_flags |= CPUFlag::AVX2;
        }
    }

    static void extract_info2(uint32_t ecx, uint32_t edx)
    {
        if (edx & (1 << 0))
        {
            cpu_flags |= CPUFlag::FPU;
        }
        if (edx & (1 << 23))
        {
            cpu_flags |= CPUFlag::MMX;
        }
        if (edx & (1 << 25))
        {
            cpu_flags |= CPUFlag::SSE;
        }
        if (edx & (1 << 26))
        {
            cpu_flags |= CPUFlag::SSE2;
        }
        if (ecx & (1 << 0))
        {
            cpu_flags |= CPUFlag::SSE3;
        }
        if (ecx & (1 << 9))
        {
            cpu_flags |= CPUFlag::SSSE3;
        }
        if (ecx & ((1 << 19) | (1 << 20)))
        {
            cpu_flags |= CPUFlag::SSE4;
        }
        if (ecx & (1 << 1))
        {
            cpu_flags |= CPUFlag::PCLMULQDQ;
        }
        if (ecx & (1 << 28))
        {
            cpu_flags |= CPUFlag::AVX;
        }
        if (ecx & (1 << 29))
        {
            cpu_flags |= CPUFlag::F16C;
        }
    }

    static void init_cpuid()
    {
        enabled = true;

        DataRegisters registers;

        invoke_cpuid(&registers, 0);

        if (registers.eax >= 1U)
        {
            invoke_cpuid(&registers, 1);
            extract_info2(registers.ecx, registers.edx);
        }

        if (registers.eax >= 7U)
        {
            invoke_cpuidex(&registers, 7, 0);
            extract_info1(registers.ebx);
        }
    }

public:
    static bool IsSupported(CPUFlag flags)
    {
        if (!enabled)
        {
            init_cpuid();
        }
        return (cpu_flags & flags) != CPUFlag::None;
    }
};
