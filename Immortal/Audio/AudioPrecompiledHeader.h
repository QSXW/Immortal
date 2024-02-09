#pragma once

#include "IAudioDevice.h"
#include "Device.h"

#ifdef _WIN32
#include "WASAPI.h"
#elif defined(__APPLE__)
#include "CoreAudio.h"
#elif defined(__linux__)
#include "ALSA.h"
#endif
