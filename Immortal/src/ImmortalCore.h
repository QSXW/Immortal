#pragma once

#ifndef __IMMORTAL_CORE_H__
#define __IMMORTAL_CORE_H__

#define UNICODE

#include "impch.h"
#include <cstdint>
#include <memory>
#include <iostream>
#include "sl.h"

#define IMMORTAL_PLATFORM_SURFACE SLSURFACE

#pragma warning( disable: 4251 )
#pragma warning( disable: 4996 )
#pragma warning( disable: 4006 )
#pragma warning( disable: 26812 )
#pragma warning( disable: 26439 )

#ifdef WINDOWS
    #if defined DLLEXPORT
        #define IMMORTAL_API __declspec(dllexport)
    #elif defined DLLIMPORT
        #define IMMORTAL_API __declspec(dllimport)
    #else
        #define IMMORTAL_API
    #endif
#else
    #error Only support Windows!
#endif

#include "Framework/Vector.h"
#include "Framework/Log.h"

namespace Immortal
{
using namespace sl;

namespace Utils
{

static void *SafeChunk;

template <class T>
constexpr inline T NullValue()
{
    return *(T*)SafeChunk;
}

}

class Exception : public std::exception
{
public: Exception(const char *what) noexcept : 
#ifdef _MSC_VER
std::exception(what, 1) { }
#elif __GNUC__
message(what) { }

    virtual const char* what() const _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_NOTHROW override
    {
        return message.c_str();
    }

    std::string message;
#endif
};
}

#endif
