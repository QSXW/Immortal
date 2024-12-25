/**
 * Copyright (C) 2022, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#include "Device.h"

#ifdef _MSC_VER
#include <immintrin.h>
#endif

namespace Immortal
{

static inline void MixAudioSamples(float *o, float *a, float *b, size_t size)
{
#if _MSC_VER
    for (size_t i = 0; i < size; i += 8)
    {
		auto _a = _mm256_load_ps(&a[i]);
		auto _b = _mm256_load_ps(&b[i]);
		auto _c = _mm256_add_ps(_a, _b);
		_c = _mm256_min_ps(_c, _mm256_set1_ps( 1.0f));
		_c = _mm256_max_ps(_c, _mm256_set1_ps(-1.0f));
		_mm256_store_ps(&o[i], _c);
    }
#else
    for (size_t i = 0; i < size; i++)
    {
		o[i] = a[i] + b[i];
    }
#endif
}

AudioDevice::AudioDevice() :
    handle{ IAudioDevice::CreateInstance() },
    pts{ 0 },
    samples{ 0 },
    stopping{ false },
    startpts{ 0 },
    reset{ true },
    status{false},
    defaultDeviceChanged{}
{
    instance = this;
    if (!handle)
    {
        return;
    }

    handle->RegisterCallback(AudioDeviceEvent_OnDefaultDeviceChanged, [=, this] {
		defaultDeviceChanged = true;
	});

    handle->RegisterCallback(AudioDeviceEvent_OnDeviceRemoved, [=, this] {

	});

  //  thread = new Thread{ [=, this] {
  //      uint64_t duration = 0;
  //      handle->Begin();

  //      const int kSamples = 1024;
		//alignas(8) StereoVector2 buffer[kSamples] = {};
		//alignas(8) StereoVector2 mixBuffer[kSamples] = {};
		//StereoVector2 *ptr = buffer;
  //      AudioFormat format = handle->GetFormat();

  //      static int lastSamples = 1024;
  //      while (!stopping)
  //      {
  //          if (callbacks.empty())
  //          {
		//		duration = Seconds2Nanoseconds(((float) kSamples / format.sampleRate));
  //          }
		//	else
  //          {
		//		std::lock_guard lock{mutex};
  //              int frames = 0;
		//	    for (size_t i = 0; i < callbacks.size(); i++)
		//	    {
		//		    auto &callback = callbacks[i];
		//		    int size = 0;
  //                  if (i > 0)
  //                  {
		//				size = callback(mixBuffer, kSamples);
  //                      if (size > 0)
  //                      {
		//					MixAudioSamples((float *) ptr, (float *) ptr, (float *) mixBuffer, kSamples * 2);
  //                      }
  //                  }
  //                  else
  //                  {
		//				size = callback(buffer, kSamples);
		//			    ptr = buffer;
  //                  }

  //                  frames = std::max(frames, size);
  //              }

		//	    auto frameLeft = PlaySamples(frames, (const uint8_t *)ptr);
		//	    duration = Seconds2Nanoseconds(((float) frameLeft / format.sampleRate));
  //          }
  //          //Picture picture{};
  //          //if (callBack)
  //          //{
  //          //    callBack(picture);
  //          //}


  //  //        if (picture)
  //  //        {
  //  //            if (reset)
  //  //            {
  //  //                startpts = picture.GetTimestamp();
  //  //                samples  = picture.GetWidth();
  //  //                reset = false;
  //  //            }
  //  //            int frameLeft = 0;
		//		//lastSamples = picture.GetWidth();
		//		//if (picture.GetWidth() < 1024)
  //  //            {
  //  //                size_t bytes = picture.GetWidth() << 3;
		//		//	memcpy(ptr, picture.GetData(), bytes);
		//		//	ptr += picture.GetWidth();
  //  //                uint32_t frames = uint32_t(ptr - buffer);
  //  //                if (frames > 1024)
  //  //                {
  //  //                    frameLeft = PlaySamples(frames, (const uint8_t *)buffer);
  //  //                    ptr = buffer;
  //  //                }
  //  //            }
  //  //            else
  //  //            {
		//		//	pts       = picture.GetTimestamp();
  //  //                samples   = picture.GetWidth();
		//		//	frameLeft = PlaySamples(samples, picture.GetData());
  //  //            }
		//		//duration = Seconds2Nanoseconds(((float) frameLeft / format.sampleRate));
  //  //        }
  //  //        else
  //  //        {
  //  //            if (lastSamples >= 512)
  //  //            {
		//		//	duration = Seconds2Nanoseconds(((float)1024 / format.sampleRate));
  //  //            }
  //  //        }

  //          duration >>= 1;
  //          std::this_thread::sleep_for(std::chrono::nanoseconds(duration));

  //          status.wait(true);
  //      }

  //      handle->End();
  //  } };
}

AudioDevice::~AudioDevice()
{
	status = false;
	status.notify_one();

	stopping = true;
    if (thread)
    {
        thread->Join();
        thread.Reset();
    }
}

void AudioDevice::OnPauseDown()
{
	status = true;
}

void AudioDevice::OnPauseRelease()
{
	status = false;
	status.notify_one();
}

void AudioDevice::Reset()
{
    reset = true;
}

double AudioDevice::GetPosition() const
{
	return 0;
}

int AudioDevice::GetSampleRate() const
{
	//AudioFormat format = handle->GetFormat();
	//return format.sampleRate;
	return 44100;
}

int AudioDevice::PlaySamples(uint32_t numberSamples, const uint8_t *pSamples)
{
  return 0;
}

IAudioStream *AudioDevice::CreateAudioStream(const PFN_AudioStreamPlayCallback &callback)
{
	IAudioStream *stream = handle->CreateStream();
    if (stream)
    {
		stream->Start(callback);
    }

    return stream;
}

AudioDevice *AudioDevice::GetInstance()
{
	return instance;
}

AudioDevice *AudioDevice::instance;

}
