/**
 * Copyright (C) 2022, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#pragma once

#ifndef ALSA_CONTEXT_H_
#define ALSA_CONTEXT_H_

#include "IAudioDevice.h"
#include <alsa/asoundlib.h>

#define ALIAS(x, y) static inline decltype(&(x)) y;

namespace Immortal
{

struct ALSA
{
    ALIAS(snd_pcm_open,                                Open                     )
    ALIAS(snd_pcm_start,                               Start                    )
    ALIAS(snd_pcm_drop,                                Drop                     )
    ALIAS(snd_pcm_reset,                               Reset                    )
    ALIAS(snd_pcm_status,                              GetStatus                )
    ALIAS(snd_pcm_drain,                               Drain                    )
    ALIAS(snd_pcm_pause,                               Pause                    )
    ALIAS(snd_pcm_prepare,                             Prepare                  )
    ALIAS(snd_strerror,                                GetErrorMessage          )
    ALIAS(snd_pcm_hw_params_malloc,                    Allocate                 )
    ALIAS(snd_pcm_hw_params_free,                      Free                     )
    ALIAS(snd_pcm_hw_params_copy,                      Copy                     )
    ALIAS(snd_pcm_hw_params,                           SetHardwareParameters    )
    ALIAS(snd_pcm_hw_params_any,                       GetHardwareParameters    )
    ALIAS(snd_pcm_hw_params_sizeof,                    GetHardwareParametersSize)
    ALIAS(snd_pcm_hw_params_set_access,                SetAccess                )
    ALIAS(snd_pcm_hw_params_set_format,                SetFormat                )
    ALIAS(snd_pcm_hw_params_set_channels,              SetChannels              )
    ALIAS(snd_pcm_hw_params_get_channels,              GetChannels              )
    ALIAS(snd_pcm_hw_params_set_rate_near,             SetRateNear              )
    ALIAS(snd_pcm_hw_params_get_buffer_size,           GetBufferSize            )
    ALIAS(snd_pcm_hw_params_set_period_size_near,      SetPeriodSizeNear        )
    ALIAS(snd_pcm_hw_params_set_periods,               SetPeriods               )
    ALIAS(snd_pcm_hw_params_set_periods_min,           SetPeriodsMin            )
    ALIAS(snd_pcm_hw_params_set_periods_max,           SetPeriodsMax            )
    ALIAS(snd_pcm_hw_params_set_periods_first,         SetPeriodsFirst          )
    ALIAS(snd_pcm_hw_params_set_periods_last,          SetPeriodsLast           )
    ALIAS(snd_pcm_sw_params,                           SetSoftwareParameters    )
    ALIAS(snd_pcm_sw_params_current,                   GetSoftwareParameters    )
    ALIAS(snd_pcm_sw_params_sizeof,                    GetSoftwareParametersSize)
    ALIAS(snd_pcm_sw_params_set_avail_min,             SetAvailableMin          )
    ALIAS(snd_pcm_sw_params_get_avail_min,             GetAvailableMin          )
    ALIAS(snd_pcm_sw_params_set_start_threshold,       SetStartThreshold        )
    ALIAS(snd_pcm_sw_params_get_start_threshold,       GetStartThreshold        )
    ALIAS(snd_pcm_readi,                               Read                     )
    ALIAS(snd_pcm_writei,                              Write                    )
    ALIAS(snd_pcm_avail,                               Avaiable                 )
};

class ALSAHardwareParameters
{
public:
    SL_OPERATOR_HANDLE(snd_pcm_hw_params_t *)

public:
    ALSAHardwareParameters();

    ~ALSAHardwareParameters();

    ALSAHardwareParameters(ALSAHardwareParameters &&other);

    ALSAHardwareParameters(const ALSAHardwareParameters &other);

    ALSAHardwareParameters &operator=(ALSAHardwareParameters &&other);

    ALSAHardwareParameters &operator=(const ALSAHardwareParameters &other);

    void CopyTo(ALSAHardwareParameters &other) const
    {
        ALSA::Copy(other, *this);
    }

    int GetChannels(uint32_t *pChannels) const
    {
        return ALSA::GetChannels(*this, pChannels);
    }

    int GetBufferSize(uint64_t *pBufferSize)
    {
        return ALSA::GetBufferSize(*this, pBufferSize);
    }

    void Swap(ALSAHardwareParameters &other)
    {
        std::swap(handle, other.handle);
    }
};

class ALSASoftwareParameters
{
public:
    SL_OPERATOR_HANDLE(snd_pcm_sw_params_t *)

public:
    ALSASoftwareParameters();

    ~ALSASoftwareParameters();

    int GetAvailableMin(snd_pcm_uframes_t *val) const
    {
        return ALSA::GetAvailableMin(*this, val);
    }

    int GetStartThreshold(snd_pcm_uframes_t *pStartThreshold)
    {
        return ALSA::GetStartThreshold(*this, pStartThreshold);
    }
};

class ALSAContext : public IAudioDevice
{
public:
    using Super = IAudioDevice;
    SL_OPERATOR_HANDLE(snd_pcm_t *)

public:
    ALSAContext();

    virtual ~ALSAContext();

    virtual void OpenDevice() override;

    virtual void Begin() override;

    virtual void End() override;

    virtual void Reset() override;

    virtual void Pause(bool enable) override;

    virtual double GetPostion() override;

    virtual void BeginRender(uint32_t frames) override;
    
    virtual void WriteBuffer(const uint8_t *buffer, size_t size) override;

    virtual void EndRender(uint32_t frames) override;

    virtual uint32_t GetAvailableFrameCount() override;
    
    virtual AudioFormat GetFormat() override;

    void Release();

    const char *GetAudioDevice();

    int SetBufferSize(const ALSAHardwareParameters &params);

public:
    int Open(const char *name, snd_pcm_stream_t stream, int mode)
    {
        return ALSA::Open(&handle, name, stream, mode);
    }

    int SetHardwareParameters(ALSAHardwareParameters &params)
    {
        return ALSA::SetHardwareParameters(handle, params);
    }

    int GetHardwareParameters(ALSAHardwareParameters &params)
    {
        snd_pcm_hw_params_t *hwparams = params;
        return ALSA::GetHardwareParameters(handle, hwparams);
    }

    int SetAccess(ALSAHardwareParameters &params, snd_pcm_access_t access)
    {
        return ALSA::SetAccess(handle, params, access);
    }

    int SetFormat(ALSAHardwareParameters &params, snd_pcm_format_t format)
    {
        return ALSA::SetFormat(handle, params, format);
    }

    int SetChannels(ALSAHardwareParameters &params, uint32_t channels)
    {
        return ALSA::SetChannels(handle, params, channels);
    }

    int SetRateNear(ALSAHardwareParameters &params, unsigned int *pSampleRate, int *dir = nullptr)
    {
        return ALSA::SetRateNear(handle, params, pSampleRate, dir);
    }

    int SetPeriodSizeNear(ALSAHardwareParameters &params, snd_pcm_uframes_t *pPeriodSize, int *dir = nullptr)
    {
        return ALSA::SetPeriodSizeNear(handle, params, pPeriodSize, dir);
    }

    int SetPeriodsMin(ALSAHardwareParameters &params, unsigned int *pPeriods, int *dir = nullptr)
    {
        return ALSA::SetPeriodsMin(handle, params, pPeriods, dir);
    }

    int SetPeriodsMax(ALSAHardwareParameters &params, unsigned int *pPeriods, int *dir = nullptr)
    {
        return ALSA::SetPeriodsMax(handle, params, pPeriods, dir);
    }

    int SetPeriodsFirst(ALSAHardwareParameters &params, unsigned int *pPeriods, int *dir = nullptr)
    {
        return ALSA::SetPeriodsFirst(handle, params, pPeriods, dir);
    }

    int SetPeriodsLast(ALSAHardwareParameters &params, unsigned int *pPeriods, int *dir = nullptr)
    {
        return ALSA::SetPeriodsLast(handle, params, pPeriods, dir);
    }

    int SetSoftwareParameters(ALSASoftwareParameters &params)
    {
        return ALSA::SetSoftwareParameters(handle, params);
    }

    int GetSoftwareParameters(ALSASoftwareParameters &params)
    {
        return ALSA::GetSoftwareParameters(handle, params);
    }

    int SetAvailableMin(ALSASoftwareParameters &params, snd_pcm_uframes_t val)
    {
        return ALSA::SetAvailableMin(handle, params, val);
    }

    int SetStartThreshold(ALSASoftwareParameters &params, snd_pcm_uframes_t startThreshold)
    {
        return ALSA::SetStartThreshold(handle, params, startThreshold);
    }

    int Write(const void *buffer, uint32_t size)
    {
        return ALSA::Write(handle, buffer, size);
    }

    int Read(void *buffer, uint32_t size)
    {
        return ALSA::Read(handle, buffer ,size);
    }

    uint32_t Avaiable()
    {
        return ALSA::Avaiable(handle);
    }

    int Prepare()
    {
        return ALSA::Prepare(handle);
    }

    int Start()
    {
        return ALSA::Start(handle);
    }

    int Drop()
    {
        return ALSA::Drop(handle);
    }

    int Drain()
    {
        return ALSA::Drain(handle);
    }

    int GetStatus(snd_pcm_status_t *pStatus)
    {
        return ALSA::GetStatus(handle, pStatus);
    }

protected:
    AudioFormat format;

    uint32_t bufferSize;
};

}

#endif
