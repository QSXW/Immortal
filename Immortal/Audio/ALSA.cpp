/**
 * Copyright (C) 2022, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#include "ALSA.h"
#include "Shared/Log.h"
#include "Shared/DLLLoader.h"

#include <array>
#include <cstdlib>

namespace Immortal
{

static DLLLoader ALSAModule;

static void InitALSALibrary()
{
    static const std::pair<void **, const char *> entries[] = {
        { (void **)&ALSA::Open,                            "snd_pcm_open"                           },
        { (void **)&ALSA::Start,                           "snd_pcm_start"                          },
        { (void **)&ALSA::Drop,                            "snd_pcm_drop"                           },
        { (void **)&ALSA::Reset,                           "snd_pcm_reset"                          },
        { (void **)&ALSA::GetStatus,                       "snd_pcm_status"                         },
        { (void **)&ALSA::Drain,                           "snd_pcm_drain"                          },
        { (void **)&ALSA::Pause,                           "snd_pcm_pause"                          },
        { (void **)&ALSA::Prepare,                         "snd_pcm_prepare"                        },
        { (void **)&ALSA::GetErrorMessage,                 "snd_strerror"                           },
        { (void **)&ALSA::Allocate,                        "snd_pcm_hw_params_malloc"               },
        { (void **)&ALSA::Free,                            "snd_pcm_hw_params_free"                 },
        { (void **)&ALSA::Copy,                            "snd_pcm_hw_params_copy"                 },
        { (void **)&ALSA::SetHardwareParameters,           "snd_pcm_hw_params"                      },
        { (void **)&ALSA::GetHardwareParameters,           "snd_pcm_hw_params_any"                  },
        { (void **)&ALSA::GetHardwareParametersSize,       "snd_pcm_hw_params_sizeof"               },
        { (void **)&ALSA::SetAccess,                       "snd_pcm_hw_params_set_access"           },
        { (void **)&ALSA::SetFormat,                       "snd_pcm_hw_params_set_format"           },
        { (void **)&ALSA::SetChannels,                     "snd_pcm_hw_params_set_channels"         },
        { (void **)&ALSA::GetChannels,                     "snd_pcm_hw_params_get_channels"         },
        { (void **)&ALSA::SetRateNear,                     "snd_pcm_hw_params_set_rate_near"        },
        { (void **)&ALSA::GetBufferSize,                   "snd_pcm_hw_params_get_buffer_size"      },
        { (void **)&ALSA::SetPeriodSizeNear,               "snd_pcm_hw_params_set_period_size_near" },
        { (void **)&ALSA::SetPeriods,                      "snd_pcm_hw_params_set_periods"          },
        { (void **)&ALSA::SetPeriodsMin,                   "snd_pcm_hw_params_set_periods_min"      },
        { (void **)&ALSA::SetPeriodsMax,                   "snd_pcm_hw_params_set_periods_max"      },
        { (void **)&ALSA::SetPeriodsFirst,                 "snd_pcm_hw_params_set_periods_first"    },
        { (void **)&ALSA::SetPeriodsLast,                  "snd_pcm_hw_params_set_periods_last"     },
        { (void **)&ALSA::SetSoftwareParameters,           "snd_pcm_sw_params"                      },
        { (void **)&ALSA::GetSoftwareParameters,           "snd_pcm_sw_params_current"              },
        { (void **)&ALSA::GetSoftwareParametersSize,       "snd_pcm_sw_params_sizeof"               },
        { (void **)&ALSA::SetAvailableMin,                 "snd_pcm_sw_params_set_avail_min"        },
        { (void **)&ALSA::GetAvailableMin,                 "snd_pcm_sw_params_get_avail_min"        },
        { (void **)&ALSA::SetStartThreshold,               "snd_pcm_sw_params_set_start_threshold"  },
        { (void **)&ALSA::GetStartThreshold,               "snd_pcm_sw_params_get_start_threshold"  },
        { (void **)&ALSA::Read,                            "snd_pcm_readi"                          },
        { (void **)&ALSA::Write,                           "snd_pcm_writei"                         },
    };

    if (!ALSAModule)
    {
        ALSAModule = std::move(DLLLoader("libasound.so"));
        if (!ALSAModule)
        {
            throw RuntimeException("Failed to load libasound.so");
        }
    }

    for (size_t i = 0; i < SL_ARRAY_LENGTH(entries); i++)
    {
         auto &[pFunc, name] = entries[i];
        *pFunc = ALSAModule.GetFunc(name);
    }
}

ALSAHardwareParameters::ALSAHardwareParameters() :
    handle{}
{
    size_t size = ALSA::GetHardwareParametersSize();
    handle = (Primitive)malloc(size);
    memset(handle, 0, size);
}

ALSAHardwareParameters::~ALSAHardwareParameters()
{
    if (handle)
    {
        free(handle);
        handle = nullptr;
    }
}

ALSAHardwareParameters::ALSAHardwareParameters(ALSAHardwareParameters &&other) :
    handle{ other.handle }
{
    other.handle = nullptr;
}

ALSAHardwareParameters::ALSAHardwareParameters(const ALSAHardwareParameters &other) :
    ALSAHardwareParameters{}
{
    other.CopyTo(*this);
}

ALSAHardwareParameters &ALSAHardwareParameters::operator=(ALSAHardwareParameters &&other)
{
    ALSAHardwareParameters(std::move(other)).Swap(*this);
    return *this;
}

ALSAHardwareParameters &ALSAHardwareParameters::operator=(const ALSAHardwareParameters &other)
{
    ALSAHardwareParameters(other).Swap(*this);
    return *this;
}

ALSASoftwareParameters::ALSASoftwareParameters()
{
    size_t size = ALSA::GetSoftwareParametersSize();
    handle = (Primitive)malloc(size);
    memset(handle, 0, size);
}

ALSASoftwareParameters::~ALSASoftwareParameters()
{
    if (handle)
    {
        free(handle);
        handle = nullptr;
    }
}

ALSAContext::ALSAContext() :
    Super{},
    handle{}
{
    OpenDevice();
}

ALSAContext::~ALSAContext()
{
    Release();
}

void ALSAContext::OpenDevice()
{
    Release();

    if (!ALSA::Open)
    {
        InitALSALibrary();
    }

    int ret = Open(GetAudioDevice(), SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK);
    if (ret < 0)
    {
        LOG::ERR("ALSA: Couldn't open audio device for {}", ALSA::GetErrorMessage(ret));
        return;
    }

    ALSAHardwareParameters hwparams;
    ret = GetHardwareParameters(hwparams);
    if (ret < 0)
    {
        LOG::ERR("ALSA: Failed to fill default hardware parameters for {}", ALSA::GetErrorMessage(ret));
        return;
    }

    ret = SetAccess(hwparams, SND_PCM_ACCESS_RW_INTERLEAVED);
    if (ret < 0)
    {
        LOG::ERR("ALSA: Couldn't set interleaved access for {}", ALSA::GetErrorMessage(ret));
        return;
    }

    ret = SetFormat(hwparams, SND_PCM_FORMAT_FLOAT_LE);
    if (ret < 0)
    {
        LOG::ERR("ALSA: The PCM format `float-pointing` doesn't support!", ALSA::GetErrorMessage(ret));
        return;
    }

    if ((ret = SetChannels(hwparams, format.Channels) < 0 &&
        (ret = hwparams.GetChannels(&format.Channels)) < 0))
    {
        LOG::ERR("ALSA: Failed to set channels for {}", ALSA::GetErrorMessage(ret));
        return;
    }

    ret = SetRateNear(hwparams, &format.SampleRate);
    if (ret < 0)
    {
        LOG::ERR("ALSA: Failed to set sample rate for {}", ALSA::GetErrorMessage(ret));
        return;
    }

    ret = SetBufferSize(hwparams);
    if (ret < 0)
    {
        LOG::ERR("ALSA: Failed to set buffer size for {}", ALSA::GetErrorMessage(ret));
        return;
    }

    ALSASoftwareParameters swparams;
    ret = GetSoftwareParameters(swparams);
    if (ret < 0)
    {
        LOG::ERR("ALSA: Failed to get software parameters for {}", ALSA::GetErrorMessage(ret));
        return;
    }

    ret = SetAvailableMin(swparams, bufferFrameCount);
    if (ret < 0)
    {
        LOG::ERR("ALSA: Failed to set buffer frame count for {}", ALSA::GetErrorMessage(ret));
        return;
    }

    ret = SetStartThreshold(swparams, 1);
    if (ret < 0)
    {
        LOG::ERR("ALSA: Failed to set start threshold for {}", ALSA::GetErrorMessage(ret));
        return;
    }

    ret = SetSoftwareParameters(swparams);
    if (ret < 0)
    {
        LOG::ERR("ALSA: Failed to set software parameters for {}", ALSA::GetErrorMessage(ret));
        return;
    }
}

void ALSAContext::Begin()
{
    int ret = Prepare();
    if (ret < 0)
    {
        LOG::ERR("ALSA: Failed to prepare for {}", ALSA::GetErrorMessage(ret));
    }

    ret = Start();
    if (ret < 0)
    {
        LOG::ERR("ALSA: Failed to start for {}", ALSA::GetErrorMessage(ret));
    }
}

void ALSAContext::End()
{
    int ret = Drop();
    if (ret < 0)
    {
        LOG::ERR("Failed to drop audio stream for `{}`", ALSA::GetErrorMessage(ret));
    }
}

void ALSAContext::Reset()
{
    int ret = ALSA::Reset(handle);
    if (ret < 0)
    {
        LOG::ERR("Failed to reset audio stream for `{}`", ALSA::GetErrorMessage(ret));
    }
}

void ALSAContext::Pause(bool enable)
{
    int ret = ALSA::Pause(handle, enable);
    if (ret < 0)
    {
        LOG::ERR("Failed to pause audio stream for `{}`", ALSA::GetErrorMessage(ret));
    }
}

int ALSAContext::PlaySamples(uint32_t numberSamples, const uint8_t *pSamples)
{
    int consumed;
    while (numberSamples > 0)
    {
        uint32_t frameRequested = std::min(numberSamples, bufferFrameCount);
        uint32_t bytes = frameRequested << 3;

        consumed = Write(pSamples, frameRequested);
        if (consumed < 0)
        {
            continue;
        }
        else
        {
            pSamples += bytes;
            numberSamples -= consumed;
        }
    }

    return consumed;
}


double ALSAContext::GetPostion()
{
    return 0.0;
}

void ALSAContext::Release()
{

}

const char *ALSAContext::GetAudioDevice()
{
    if (handle)
    {
        return (const char *)handle;
    }

    const char *device = std::getenv("AUDIODEV");
    if (device)
    {
        return device;
    }

    return device ? device : "default";
}

int ALSAContext::SetBufferSize(const ALSAHardwareParameters &params)
{
    ALSAHardwareParameters hwparams;
    params.CopyTo(hwparams);

    snd_pcm_uframes_t periodSize;
    int ret = SetPeriodSizeNear(hwparams, &periodSize, nullptr);
    if (ret < 0)
    {
        return ret;
    }

    uint32_t periods = 2;
    ret = SetPeriodsMin(hwparams, &periods);
    if (ret < 0)
    {
        return ret;
    }

    ret = SetPeriodsFirst(hwparams, &periods);
    if (ret < 0)
    {
        return ret;
    }

    ret = SetHardwareParameters(hwparams);
    bufferFrameCount = periodSize;

    return 0;
}

}
