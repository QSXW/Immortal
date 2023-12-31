#pragma once

#ifndef __IMMORTAL_CORE_H__
#define __IMMORTAL_CORE_H__

#include <cstdint>
#include <memory>
#include <iostream>
#include <algorithm>

#include "slapi.h"

#pragma warning( disable: 4251  )
#pragma warning( disable: 4996  )
#pragma warning( disable: 4006  )
#pragma warning( disable: 26812 )
#pragma warning( disable: 26439 )

#ifdef  _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

#ifdef _MSC_VER
    #if defined EXPORT_IMMORTAL
        #define IMMORTAL_API __declspec(dllexport)
    #elif defined IMPORT_IMMORTAL
        #define IMMORTAL_API __declspec(dllimport)
    #else
        #define IMMORTAL_API
    #endif
#elif defined(__GNUC__) || defined(__clang__)
    #define IMMORTAL_API __attribute__((visibility("default")))
#else
    #error Unsupport Platform Detected!
#endif

#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif

#define AUTO_DEVICE_ID -1

namespace Immortal
{

using namespace sl;

}
#endif
