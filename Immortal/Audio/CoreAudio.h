/**
 * Copyright (C) 2024, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#pragma once


#ifndef COREAUDIO_CONTEXT_H_
#define COREAUDIO_CONTEXT_H_

#include "Core.h"
#include "Shared/IObject.h"
#include "Shared/Async.h"
#include "IAudioDevice.h"
#include <CoreAudio/CoreAudio.h>
#include <AudioToolbox/AudioToolbox.h>
#include <AudioToolbox/AudioQueue.h>
#include <AudioUnit/AudioUnit.h>
#include <algorithm>
#include <queue>
#include <vector>
#include <mutex>

namespace Immortal
{
namespace CoreAudio
{

struct AudioBuffer
{
    uint8_t *data;
    size_t   size;
};

class Device : public IAudioDevice
{
public:
    using Super = IAudioDevice;
    SL_SWAPPABLE(Device)

public:
    Device();

    virtual ~Device() override;

    virtual void OpenDevice() override;

    virtual void Begin() override;

    virtual void End() override;

    virtual void Reset() override;

    virtual void Pause(bool enable) override;

    virtual double GetPostion() override;

    virtual void BeginRender(uint32_t frames) override;

    virtual void EndRender(uint32_t frames) override;

    virtual void WriteBuffer(const uint8_t *buffer, size_t size) override;

    virtual uint32_t GetAvailableFrameCount() override;

    virtual AudioFormat GetFormat() override;

    void RefreshPhysicalDevices();

    void CheckDevice(std::vector<AudioDeviceID> &devices, AudioDeviceType deviceType);

    void ConnectDeviceToAudioQueue(AudioDeviceType type);

    void OnBufferReady(AudioQueueRef audioQueue, AudioQueueBufferRef buffer);

    void Release();

public:
    void Swap(Device &other)
    {
        std::swap_ranges(&handle,   &handle   + SL_ARRAY_LENGTH(handle),   &other.handle  );
        std::swap_ranges(&channels, &channels + SL_ARRAY_LENGTH(channels), &other.channels);

        std::swap(queue,           other.queue          );
        std::swap(buffers,         other.buffers        );
        std::swap(description,     other.description    );
        std::swap(availableFrames, other.availableFrames);
        std::swap(framesPerBuffer, other.framesPerBuffer);
        std::swap(memoryResource,  other.memoryResource );
        std::swap(bufferQueue,     other.bufferQueue    );
        std::swap(data,            other.data           );
        std::swap(mutex,           other.mutex          );
    }

protected:
    AudioDeviceID handle[2];

    AudioQueueRef queue;

    std::vector<AudioQueueBufferRef> buffers;

    AudioStreamBasicDescription description;

    uint32_t availableFrames;

    uint32_t framesPerBuffer;

    std::vector<uint8_t> memoryResource;

    std::queue<AudioBuffer> bufferQueue;

    uint8_t *data;

    URef<std::mutex> mutex;

    uint8_t channels[2];
};

}
}

#endif
