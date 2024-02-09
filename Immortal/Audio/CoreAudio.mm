/**
 * Copyright (C) 2024, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#include "CoreAudio.h"
#include <CoreAudio/CoreAudio.h>
#include <AudioToolbox/AudioToolbox.h>
#include <AudioUnit/AudioUnit.h>
#include <MacTypes.h>
#include "Audio/IAudioDevice.h"
#include "Shared/Log.h"
#include "Shared/Async.h"
#include "Graphics/Format.h"

namespace Immortal
{

namespace CoreAudio
{

static const AudioObjectPropertyAddress DevicesAddress = {
    .mSelector = kAudioHardwarePropertyDevices,
    .mScope    = kAudioObjectPropertyScopeGlobal,
    .mElement  = kAudioObjectPropertyElementMain
};

static AudioChannelLayoutTag GetAudioChannelLayout(int channels)
{
    switch (channels)
    {
        case 1:
            return kAudioChannelLayoutTag_Mono;
            break;
        case 2:
            return kAudioChannelLayoutTag_Stereo;

        case 3:
            return kAudioChannelLayoutTag_DVD_4;

        case 4:
            return kAudioChannelLayoutTag_Quadraphonic;

        case 5:
            return kAudioChannelLayoutTag_MPEG_5_0_A;

        case 6:
            return kAudioChannelLayoutTag_MPEG_5_1_A;

        case 7:
            return kAudioChannelLayoutTag_MPEG_6_1_A;

        case 8:
            return kAudioChannelLayoutTag_MPEG_7_1_A;

        default:
            return kAudioChannelLayoutTag_Unknown;
    }
}

static void OnBufferReadyCallback(void *data, AudioQueueRef audioQueue, AudioQueueBufferRef buffer)
{
    Device *This = (Device *)data;
    This->OnBufferReady(audioQueue, buffer);
}

static OSStatus DeviceListChangedNotification(AudioObjectID systemObj, UInt32 num_addr, const AudioObjectPropertyAddress *addrs, void *data)
{
    Device *This = (Device *)data;
    This->RefreshPhysicalDevices();
    return kAudioHardwareNoError;
}

Device::Device() :
    Super{},
    handle{},
    queue{},
    buffers{},
    description{},
    availableFrames{},
    framesPerBuffer{},
    memoryResource{},
    bufferQueue{},
    data{},
    mutex{ new std::mutex{} },
    channels{}
{
    OpenDevice();
}

Device::~Device()
{
    Release();
}

void Device::OpenDevice()
{
    Release();
    RefreshPhysicalDevices();

    AudioObjectPropertyAddress defaultOutputDeviceAddress = {
        .mSelector = kAudioHardwarePropertyDefaultOutputDevice,
        .mScope    = kAudioObjectPropertyScopeGlobal,
        .mElement  = kAudioObjectPropertyElementMain
    };

    AudioObjectAddPropertyListener(kAudioObjectSystemObject, &DevicesAddress, DeviceListChangedNotification, (void *)this);

    AudioDeviceID device = handle[0];

    AudioDeviceID defaultDevice = 0;
    uint32_t size = sizeof(AudioDeviceID);
    OSStatus status = AudioObjectGetPropertyData(kAudioObjectSystemObject, &defaultOutputDeviceAddress, 0, nullptr, &size, &defaultDevice);
    if (status != kAudioHardwareNoError)
    {
        LOG::ERR("CoreAudio: failed to query the default output device!");
        return;
    }

    description = AudioStreamBasicDescription {
        .mSampleRate       = description.mSampleRate,
        .mFormatID         = kAudioFormatLinearPCM,
        .mFormatFlags      = kLinearPCMFormatFlagIsPacked | kLinearPCMFormatFlagIsFloat,
        .mFramesPerPacket  = 1,
        .mChannelsPerFrame = channels[0],
    };

    description.mBitsPerChannel = sizeof(float) * 8;
    description.mBytesPerFrame  = description.mChannelsPerFrame * description.mBitsPerChannel / 8;
    description.mBytesPerPacket = description.mBytesPerFrame * description.mFramesPerPacket;

    AudioObjectPropertyAddress address = {
        .mSelector = kAudioDevicePropertyDeviceIsAlive,
        .mScope    = kAudioDevicePropertyScopeOutput,
        .mElement  = kAudioObjectPropertyElementMain
    };

    uint32_t alive  = 0;
    size   = sizeof(alive);
    status = AudioObjectGetPropertyData(device, &address, 0, nullptr, &size, &alive);

    if (status == kAudioHardwareNoError && !alive)
    {
        LOG::ERR("CoreAudio: requested device existed but not alive!");
        return;
    }

    pid_t pid = 0;
    size = sizeof(pid);
    address.mSelector = kAudioDevicePropertyHogMode;
    status = AudioObjectGetPropertyData(device, &address, 0, nullptr, &size, &pid);
    if (status == kAudioHardwareNoError && pid != -1)
    {
        LOG::ERR("CoreAudio: requested device is hogged");
    }

    status = AudioQueueNewOutput(&description, OnBufferReadyCallback, (void *)this, CFRunLoopGetCurrent(), kCFRunLoopCommonModes, 0, &queue);
    if (status != kAudioHardwareNoError)
    {
        LOG::ERR("CoreAudio::AudioQueueNewOutput: failed to allocate output!");
        return;
    }

    ConnectDeviceToAudioQueue(AudioDeviceType::Ouput);

    AudioChannelLayout layout = {};
    layout.mChannelLayoutTag = GetAudioChannelLayout(description.mChannelsPerFrame);
    if (layout.mChannelLayoutTag != kAudioChannelLayoutTag_Unknown)
    {
        status = AudioQueueSetProperty(queue, kAudioQueueProperty_ChannelLayout, &layout, sizeof(layout));
        if (status != kAudioHardwareNoError)
        {
            LOG::ERR("CoreAudio::AudioQueue: failed to set property - channel layout!");
        }
    }

    Format format           = Format::VECTOR2;
    framesPerBuffer         = 1024;
    size_t bufferSize       = framesPerBuffer * format.GetTexelSize();

    size_t audioBufferCount = 2;
    buffers.resize(audioBufferCount);
    for (auto &buffer : buffers)
    {
        status = AudioQueueAllocateBuffer(queue, (uint32_t)bufferSize, &buffer);
        if (status != kAudioHardwareNoError)
        {
            LOG::ERR("CoreAudio::AudioQueue: failed to allocate buffer!");
            return;
        }
        memset(buffer->mAudioData, 0, buffer->mAudioDataBytesCapacity);
        buffer->mAudioDataByteSize = buffer->mAudioDataBytesCapacity;
        status = AudioQueueEnqueueBuffer(queue, buffer, 0, nullptr);
        if (status != kAudioHardwareNoError)
        {
            LOG::ERR("CoreAudio::AudioQueue: failed to enqueue buffer!");
            return;
        }
    }

    status = AudioQueueStart(queue, nullptr);
    if (status != kAudioHardwareNoError)
    {
        LOG::ERR("CoreAudio: faild to start audio queue!");
    }

    memoryResource.resize(format.GetTexelSize() * description.mSampleRate * 2);
}

void Device::Begin()
{
    AudioQueueStart(queue, 0);
}

void Device::End()
{
    AudioQueueStop(queue, 0);
}

void Device::Reset()
{
    {
        std::lock_guard lock{ *mutex };
        bufferQueue = {};
        availableFrames = 0;
    }
    AudioQueueFlush(queue);
}

void Device::Pause(bool enabled)
{
    if (enabled)
    {
        AudioQueuePause(queue);
    }
    else
    {
        AudioQueueStart(queue, nullptr);
    }
}

double Device::GetPostion()
{
    return 0.0f;
}

void Device::BeginRender(uint32_t frames)
{
    mutex->lock();
    data = &memoryResource.back() - availableFrames * description.mBytesPerFrame;
    availableFrames -= frames;
}

void Device::EndRender(uint32_t frames)
{
    bufferQueue.push(AudioBuffer{ (uint8_t *)data, frames * description.mBytesPerFrame });
    data = nullptr;
    mutex->unlock();
}

void Device::WriteBuffer(const uint8_t *buffer, size_t size)
{
    memcpy(data, buffer, size);
}

uint32_t Device::GetAvailableFrameCount()
{
    if (availableFrames == 0)
    {
        availableFrames = description.mSampleRate * 2;
    }

    return availableFrames;
}

AudioFormat Device::GetFormat()
{
    AudioFormat ret = {
        .format     = Format::VECTOR2,
        .channels   = description.mChannelsPerFrame,
        .silence    = 0,
        .sampleRate = description.mSampleRate,
    };

    return ret;
}

void Device::RefreshPhysicalDevices()
{
    uint32_t size = 0;

    OSStatus status = AudioObjectGetPropertyDataSize(kAudioObjectSystemObject, &DevicesAddress, 0, nullptr, &size);
    if (status != kAudioHardwareNoError)
    {
        return;
    }

    uint32_t deviceCount = size / sizeof(AudioDeviceID);
    std::vector<AudioDeviceID> devices;
    devices.resize(deviceCount);
    status = AudioObjectGetPropertyData(kAudioObjectSystemObject, &DevicesAddress, 0, nullptr, &size, devices.data());
    if (status != kAudioHardwareNoError)
    {
        return;
    }

    CheckDevice(devices, AudioDeviceType::Ouput);
    CheckDevice(devices, AudioDeviceType::Capture);
}

void Device::CheckDevice(std::vector<AudioDeviceID> &devices, AudioDeviceType deviceType)
{
    AudioObjectPropertyScope scope = deviceType == AudioDeviceType::Capture ? kAudioDevicePropertyScopeInput : kAudioDevicePropertyScopeOutput;
    AudioObjectPropertyElement element = kAudioObjectPropertyElementMain;

    AudioObjectPropertyAddress address = {
        .mSelector = kAudioDevicePropertyStreamConfiguration,
        .mScope    = scope,
        .mElement  = element,
    };

    AudioObjectPropertyAddress nameAddress = {
        .mSelector = kAudioObjectPropertyName,
        .mScope    = scope,
        .mElement  = element,
    };

    AudioObjectPropertyAddress sampleRateAddress = {
        .mSelector = kAudioDevicePropertyNominalSampleRate,
        .mScope    = scope,
        .mElement  = element,
    };

    std::vector<AudioBufferList> audioBufferLists;
    for (size_t i = 0; i < devices.size(); i++)
    {
        auto &device = devices[i];
        if (device == 0)
        {
            continue;
        }

        uint32_t size = 0;
        OSStatus status = AudioObjectGetPropertyDataSize(device, &address, 0, nullptr, &size);
        if (status != kAudioHardwareNoError)
        {
            continue;;
        }

        std::unique_ptr<AudioBufferList> audioBufferList;
        audioBufferList.reset((AudioBufferList *)new uint8_t[size]);

        status = AudioObjectGetPropertyData(device, &address, 0, nullptr, &size, audioBufferList.get());
        if (status != kAudioHardwareNoError)
        {
            return;
        }

        auto &channel = channels[(int)deviceType];
        for (int j = 0; j < audioBufferList->mNumberBuffers; j++)
        {
            channel += audioBufferList->mBuffers[j].mNumberChannels;
        }
        audioBufferList.reset();

        if (channel == 0)
        {
            continue;
        }

        size = sizeof(description.mSampleRate);
        status = AudioObjectGetPropertyData(device, &sampleRateAddress, 0, nullptr, &size, &description.mSampleRate);
        if (status != kAudioHardwareNoError)
        {
            LOG::ERR("CoreAudio: failed to query sample rate!");
        }

        CFStringRef stringRef = nullptr;
        size = sizeof(CFStringRef);
        status = AudioObjectGetPropertyData(device, &nameAddress, 0, nullptr, &size, &stringRef);
        if (status != kAudioHardwareNoError)
        {
            LOG::ERR("CoreAudio: failed to query device name!");
            continue;
        }

        CFIndex length = CFStringGetMaximumSizeForEncoding(CFStringGetLength(stringRef), kCFStringEncodingUTF8);
        std::string name;
        name.resize(length);
        if (CFStringGetCString(stringRef, name.data(), name.size(), kCFStringEncodingUTF8))
        {
            while (length > 0 && name[length - 1] == ' ')
            {
                length--;
            }
            if (length < name.size())
            {
                name[length] = '\0';
            }

            LOG::INFO("CoreAudio: found {} device[{}] - {} - {}", deviceType == AudioDeviceType::Capture ? "capture" : "output", i, name, (int)device);
        }
        CFRelease(stringRef);

        handle[(int)deviceType] = device;
        device = 0;
    }
}

void Device::ConnectDeviceToAudioQueue(AudioDeviceType type)
{
    AudioDeviceID device = handle[(int)type];
    const AudioObjectPropertyAddress deviceUIDAddress = {
        kAudioDevicePropertyDeviceUID,
        type == AudioDeviceType::Capture ? kAudioDevicePropertyScopeInput : kAudioDevicePropertyScopeOutput,
        kAudioObjectPropertyElementMain
    };

    CFStringRef deviceUID = {};
    uint32_t size = sizeof(CFStringRef);
    if (AudioObjectGetPropertyData(device, &deviceUIDAddress, 0, nullptr, &size, &deviceUID) != kAudioHardwareNoError ||
        AudioQueueSetProperty(queue, kAudioQueueProperty_CurrentDevice, &deviceUID, size) != kAudioHardwareNoError)

    {
        LOG::ERR("CoreAudio: failed to connect device to the audio queue!");
        return;
    }

    return;
}

void Device::OnBufferReady(AudioQueueRef audioQueue, AudioQueueBufferRef buffer)
{
    buffer->mAudioDataByteSize = 0;
    while (true)
    {
        mutex->lock();
        if (bufferQueue.empty())
        {
            mutex->unlock();
            break;
        }
        auto &b = bufferQueue.front();
        mutex->unlock();

        uint8_t *data = (uint8_t *)buffer->mAudioData;
        data += buffer->mAudioDataByteSize;

        auto copySize = std::min(b.size, (size_t)buffer->mAudioDataBytesCapacity - buffer->mAudioDataByteSize);
        memcpy(data, b.data, copySize);
        buffer->mAudioDataByteSize += copySize;
        b.size -= copySize;
        b.data += copySize;

        if (b.size == 0)
        {
            mutex->lock();
            bufferQueue.pop();
            mutex->unlock();
        }
        if (buffer->mAudioDataByteSize == buffer->mAudioDataBytesCapacity)
        {
            break;
        }
    }

    if (buffer->mAudioDataByteSize == 0)
    {
        buffer->mAudioDataByteSize = buffer->mAudioDataBytesCapacity;
        memset(buffer->mAudioData, 0, buffer->mAudioDataBytesCapacity);
    }
    AudioQueueEnqueueBuffer(queue, buffer, 0, nullptr);
}

void Device::Release()
{
    if (queue)
    {
        for (auto &buffer : buffers)
        {
            AudioQueueFreeBuffer(queue, buffer);
        }
        buffers.clear();

        AudioQueueFlush(queue);
        AudioQueueStop(queue, 0);
        AudioQueueDispose(queue, 0);
    }

    std::lock_guard lock{ *mutex };
    bufferQueue = {};
    memoryResource = {};
}

}
}
