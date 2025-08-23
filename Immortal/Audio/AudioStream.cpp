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
    exited{},
    bytePerSample{}
{

}

AudioStream::~AudioStream()
{

}

void AudioStream::Destroy()
{
	exited = true;
    thread.Join();
	thread = {};
}

void AudioStream::Start(const PFN_AudioStreamPlayCallback &value)
{
    callback = value;
    thread = [=, this] {
        Start();
        uint64_t duration = 0;
        AudioFormat format = GetFormat();    

        int samples = format.sampleRate * 0.02;
		size_t bufferSize = samples * format.format.GetTexelSize() * format.channels;
		URef<uint8_t> buffer = new uint8_t[bufferSize];

        while (!exited)
        {
            std::lock_guard lock{mutex};
            int frames = 0;
			int size = callback(buffer, samples);
            if (size > 0)
            {
				PlaySamples(size, (const uint8_t *) buffer);
            }
            else
            {
				std::this_thread::sleep_for(std::chrono::nanoseconds(1));
            }
        }

        CLOG_DEBUG("Exited {}", name);
    };
}

int AudioStream::PlaySamples(uint32_t numberSamples, const uint8_t *pSamples)
{
	AudioFormat format = GetFormat(); 
	uint32_t sampleRequested = 0;
    while (numberSamples > 0)
    {
        uint32_t numFramesPadding = GetAvailableFrameCount();

        sampleRequested = std::min(numberSamples, numFramesPadding);
		if (sampleRequested > 0)
        {
			BeginRender(sampleRequested);

            uint32_t bytes = bytePerSample * sampleRequested;
            WriteBuffer(pSamples, bytes);
            pSamples += bytes;
			numberSamples -= sampleRequested;
			EndRender(sampleRequested);
			uint64_t duration = Seconds2Nanoseconds(((float) sampleRequested / format.sampleRate)) / 2;
			std::this_thread::sleep_for(std::chrono::nanoseconds(duration));
        }
    }

    return sampleRequested;
}

void AudioStream::SetDebugName(const std::string &value)
{
	name = value;
	thread.SetDebugDescription(value);
}

}
