#pragma once

#ifndef __IMMORTAL_CORE_H__
#define __IMMORTAL_CORE_H__

#define UNICODE

#include "impch.h"
#include <cstdint>
#include <memory>
#include <iostream>
#include <algorithm>

#include "sl.h"

#define IMMORTAL_PLATFORM_SURFACE SLSURFACE

#pragma warning( disable: 4251  )
#pragma warning( disable: 4996  )
#pragma warning( disable: 4006  )
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

#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif

#include "Framework/Log.h"

namespace Immortal
{

using namespace sl;

}
#endif
