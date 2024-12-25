#include "AudioStream.h"

namespace Immortal
{

struct StereoVector2
{
    float x;
    float y;
};

static inline float Seconds2Nanoseconds(float seconds)
{
    return seconds * 1000000000;
}

AudioStream::AudioStream() :
    thread{},
    exited{}
{

}

AudioStream::~AudioStream()
{
    exited = true;
    if (thread.joinable())
    {
		thread.join();
    }
}

void AudioStream::Start(const PFN_AudioStreamPlayCallback &value)
{
    callback = value;
    thread = std::thread{[=, this] {
        Start();
        uint64_t duration = 0;

        const int kSamples = 1024;
        alignas(8) StereoVector2 buffer[kSamples] = {};
        AudioFormat format = GetFormat();

        while (!exited)
        {
            std::lock_guard lock{mutex};
            int frames = 0;
            int size = callback(buffer, kSamples);
            if (size > 0)
            {
                frames = std::max(frames, size);
                auto frameLeft = PlaySamples(frames, (const uint8_t *)buffer);
                duration = Seconds2Nanoseconds(((float) frameLeft / format.sampleRate));
            }

            duration /= 2;
            std::this_thread::sleep_for(std::chrono::nanoseconds(duration));
        }
    }};
}

int AudioStream::PlaySamples(uint32_t numberSamples, const uint8_t *pSamples)
{
    uint32_t frameRequested = 0;
    while (numberSamples > 0)
    {
        uint32_t numFramesPadding = GetAvailableFrameCount();

        frameRequested = std::min(numberSamples, numFramesPadding);
        if (frameRequested > 0)
        {
            BeginRender(frameRequested);

            uint32_t bytes = frameRequested << 3;
            WriteBuffer(pSamples, bytes);
            pSamples += bytes;
            numberSamples -= frameRequested;
            EndRender(frameRequested);
        }
    }

    return frameRequested;
}

}
