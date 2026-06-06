/**
 * Copyright (C) 2022-2024, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#include "VideoPlayerComponent.h"
#include "Audio/Device.h"
#include "Config.h"

#include <shared_mutex>

#define IMMORTAL_HAVE_VIDEO_PLAYER_STATISTIC 1

namespace Immortal
{

struct VideoPlayerContext
{
public:
	VideoPlayerContext(Ref<Demuxer> demuxer, Ref<VideoCodec> decoder, Ref<VideoCodec> audioDecoder = nullptr, Ref<VideoCodec> subtitleDecoder = nullptr, int cacheSize = 3, const VideoDecodeCallbacks &callbacks = {});

    ~VideoPlayerContext();

    VideoPlayerContext(const VideoPlayerContext &&other) = delete;

    VideoPlayerContext &operator=(const VideoPlayerContext &&other) = delete;

    void Seek(double seconds, int64_t min, int64_t max);

    Picture GetPicture();

    Picture GetAudioFrame();

    void PopPicture();

    void PopAudioFrame();

    void StartPlay();

    uint32_t GetAudioData(uint8_t *data, uint32_t samples);

	uint32_t WriteAudioData(void *data, uint32_t samples);

public:
    const Vision::DisplayOrientation *GetDisplayOrientation() const
    {
		return decoder->GetProperty<Vision::DisplayOrientation>();
    }

    const String &GetSource() const
    {
        return demuxer->GetSource();
    }

    bool IsEof() const
    {
		return eof;
    }

    void SetPause(bool enabled)
    {
		pause = enabled;
        if (audioStream)
        {
			audioStream->Stop();
			audioStream->Reset();
			audioStream->Start();
        }
    }

    void EnumerateTracks(MediaType mediaType, std::vector<Vision::TrackInfo> &tracks)
    {
		demuxer.InterpretAs<Vision::FFDemuxer>()->EnumerateTracks(mediaType, tracks);
    }

    CodecError SwitchTrack(MediaType mediaType, int index)
	{
		return demuxer.InterpretAs<Vision::FFDemuxer>()->SwitchTrack(mediaType, index);
    }

    Picture GetCurrentAudioFrame() const
    {
		return outputAudioFrame;
    }

    uint64_t GetUnconsumedSamples() const
    {
		return unconsumedSamples;
    }

    double &GetExternalClock()
    {
		return externalClock;
    }

public:
    URef<Thread> demuxerThread;

    std::atomic_bool decoding = false;

    ConcurrentQueue<CodedFrame> codedFrames;

    ConcurrentQueue<void *> memory;

    std::unique_ptr<ThreadPool> videoThreadPool;

    std::unique_ptr<ThreadPool> audioThreadPool;

    std::unique_ptr<ThreadPool> subtitleThreadPool;

    URef<AudioStream> audioStream;

    struct
    {
        std::mutex demux;

        std::shared_mutex video;

        std::shared_mutex audio;
    } mutex;

    std::condition_variable condition;

    Ref<VideoCodec> decoder;

    Ref<VideoCodec> audioDecoder;
    
    Ref<VideoCodec> subtitleDecoder;

    Ref<Demuxer> demuxer;

    ConcurrentQueue<Picture> pictures;

    ConcurrentQueue<Picture> audioFrames;

    ConcurrentQueue<Picture> subtitles;

    Picture picture;

    Picture audioFrame;
    
    Picture outputAudioFrame;

    const int kCacheSize;

    struct State
    {
        bool playing = false;
        bool exited = false;
        bool flush = false;
    } state;

    double time = 0.0f;

    int frames = 0;

    int audioSize = 0;

    int subtitleSize = 0;

    int eof = false;

    uint32_t unconsumedSamples = 0;

    std::atomic_int pictureSize = 0;

    VideoDecodeCallbacks callbacks{};

    bool pause = false;

    double externalClock = -1.0f;

    int64_t lastAudioTimestamp = 0;

#if IMMORTAL_HAVE_VIDEO_PLAYER_STATISTIC
	Timer timer;
#endif
};

CodecError AsyncDecode(const Vision::CodedFrame &codedFrame, Vision::Interface::Codec *decoder)
{
    return decoder->Decode(codedFrame);
}

void EndOfFile(Vision::Interface::Codec *decoder, const std::function<void(Picture &&)> &callback, int &frames)
{
	decoder->Flush();
	Picture picture{};
    while (decoder->GetPicture(picture) == CodecError::Success)
    {
		callback(std::move(picture));
#if IMMORTAL_HAVE_VIDEO_PLAYER_STATISTIC
		frames++;
#endif
    }

    picture = Picture{ 0, 0, Format::None };
	picture.SetFlags(Vision::PictureFlags::Eof);					
    callback(std::move(picture));
}

void VideoPlayerContext::StartPlay()
{
	demuxerThread->Start();
	timer.Start();
	demuxerThread->SetDescription("VideoDemux");
}

VideoPlayerContext::VideoPlayerContext(Ref<Demuxer> demuxer, Ref<VideoCodec> decoder, Ref<VideoCodec> audioDecoder, Ref<VideoCodec> subtitleDecoder, int cacheSize, const VideoDecodeCallbacks &callbacks) :
    demuxerThread{},
    videoThreadPool{ new ThreadPool{1} },
    audioThreadPool{ audioDecoder ? new ThreadPool{1} : nullptr },
    subtitleThreadPool{ subtitleDecoder ?new ThreadPool{1} : nullptr },
    decoder{decoder},
    audioDecoder{ audioDecoder },
    subtitleDecoder{ subtitleDecoder },
    demuxer{demuxer},
    state{},
    kCacheSize{cacheSize},
    callbacks{ callbacks },
    timer{}
{
	if (!callbacks.VideoDecodeFinishSlot)
	{
		if (audioDecoder)
		{
			AudioDevice *audioDevice = AudioDevice::GetInstance();
			if (audioDevice)
			{
				audioStream = audioDevice->CreateAudioStream([=, this](void *data, uint32_t samples) -> uint32_t {
					return GetAudioData((uint8_t *)data, samples);
				});
				audioStream->SetDebugName(demuxer->GetSource().GetString());
				audioDecoder.InterpretAs<Vision::FFCodec>()->SetSampleRate(audioStream->GetFormat().sampleRate);
			}
			else
			{
				LOG::ERR("No audio device available!");
			}
		}

        demuxerThread = new Thread{[=, this]() {
        while (true)
        {
            std::unique_lock lock{ mutex.demux };
            condition.wait(lock, [this] {
				return state.exited || ((pictureSize + videoThreadPool->TaskSize()) <= kCacheSize);
            });

            if (state.exited)
            {
				double time = timer.Duration();
				LOG::INFO("Decoding Statistic: frames:{}, time:{}, fps:{}", frames, time, frames / time);
                break;
            }

			Vision::CodedFrame codedFrame;
			auto ret = demuxer->Read(&codedFrame);
			if (ret != CodecError::Success)
			{
                if (ret == CodecError::EndOfFile)
                {
					eof = true;
                }
				continue;
			}

            switch (codedFrame.GetType())
            {
			case MediaType::Video:
            {
				videoThreadPool->Enqueue([=, this]() -> void {
					AsyncDecode(codedFrame, decoder);
                    Picture picture{};
                    while (decoder->GetPicture(picture) == CodecError::Success)
                    {
#if IMMORTAL_HAVE_VIDEO_PLAYER_STATISTIC
						frames++;
#endif
                        if (pictures.enqueue(picture))
                        {
							pictureSize++;
                        }
                        else
                        {
							LOG::ERR("Failed to enqueue picture into pending queue for out of memory");
                        }
					}
				});
				break;
            }
			case MediaType::Audio:
            {
                if (audioDecoder)
                {
					audioThreadPool->Enqueue([=, this]() -> void {
						AsyncDecode(codedFrame, audioDecoder);
						Picture picture{};
						while (audioDecoder->GetPicture(picture) == CodecError::Success)
						{
                            if (audioFrames.enqueue(picture))
                            {
#if IMMORTAL_HAVE_VIDEO_PLAYER_STATISTIC
								audioSize++;
#endif
                            }
							else
							{
								LOG::ERR("Failed to enqueue picture into pending queue for out of memory");
							}
						}
					});
                }
			    break;
            }

            case MediaType::Subtitle:
            {
                if (subtitleDecoder)
                {
//					subtitleThreadPool->Enqueue([=, this] {
// 						AsyncDecode(codedFrame, subtitleDecoder);
//						Picture picture{};
//						while (subtitleDecoder->GetPicture(picture) == CodecError::Success)
//						{
//							if (subtitles.enqueue(picture))
//							{
//#if IMMORTAL_HAVE_VIDEO_PLAYER_STATISTIC
//								subtitleSize++;
//#endif
//							}
//						}
//					});
                }
				break;
            }

            default:
				break;
            }
        }
        }};
		StartPlay();
    }
	else
    {
		videoThreadPool->OnNotify([=, this] {
			condition.notify_one();
		});

        demuxerThread = new Thread{[=, this]() {
            while (true)
            {
                std::unique_lock lock{ mutex.demux };
                condition.wait(lock, [=, this] {
					bool hasTask  = videoThreadPool->TaskSize() <= kCacheSize;
					return state.exited || eof || hasTask;
                });

                if (state.exited)
                {
				    double time = timer.Duration();
				    LOG::INFO("Decoding Statistic: frames:{}, time:{}, fps:{}", frames, time, frames / time);
                    break;
                }

			    Vision::CodedFrame codedFrame;
			    auto ret = demuxer->Read(&codedFrame);
			    if (ret != CodecError::Success)
			    {
                    if (ret == CodecError::EndOfFile)
                    {
                        if (!eof)
                        {
							videoThreadPool->Enqueue([=, this] {
								EndOfFile(decoder, callbacks.VideoDecodeFinishSlot, frames);
							});
							audioThreadPool->Enqueue([=, this]() -> void {
								EndOfFile(audioDecoder, callbacks.AudioDecodeFinishSlot, audioSize);
							});
                        }
						eof = true;
                    }
				    continue;
			    }

                switch (codedFrame.GetType())
                {
			    case MediaType::Video:
                {
				    videoThreadPool->Enqueue([=, this]() -> void {
					    AsyncDecode(codedFrame, decoder);
						Vision::Picture picture;
						while (decoder->GetPicture(picture) == CodecError::Success)
					    {
							callbacks.VideoDecodeFinishSlot(std::move(picture));
    #if IMMORTAL_HAVE_VIDEO_PLAYER_STATISTIC
						    frames++;
    #endif
					    }
				    });
				    break;
                }
			    case MediaType::Audio:
                {
                    if (audioDecoder)
                    {
					    audioThreadPool->Enqueue([=, this]() -> void {
						    AsyncDecode(codedFrame, audioDecoder);
							Vision::Picture picture;
							while (audioDecoder->GetPicture(picture) == CodecError::Success)
						    {
							    callbacks.AudioDecodeFinishSlot(std::move(picture));
    #if IMMORTAL_HAVE_VIDEO_PLAYER_STATISTIC
								audioSize++;
    #endif
						    }
					    });
                    }
			        break;
                }

                case MediaType::Subtitle:
				    break;

                default:
				    break;
                }
            }
        }};
    }
}

void VideoPlayerContext::Seek(double seconds, int64_t min, int64_t max)
{
	audioStream->Stop();
	audioStream->Reset();

    std::unique_lock lock{ mutex.demux };
    videoThreadPool->RemoveTasks();
    audioThreadPool->RemoveTasks();
    videoThreadPool->Join();
    audioThreadPool->Join();

    {
		ConcurrentQueue<Picture> _empty;
		pictures.swap(_empty);
		picture = {};
		pictureSize = 0;
    }

    {

        ConcurrentQueue<Picture> _audioFrames;
		audioFrames.swap(_audioFrames);
		audioFrame = {};
		audioSize = 0;
    }

    eof = false;
    demuxer->Seek(MediaType::Video, seconds, min, max);
    condition.notify_all();
	audioStream->Start();
}

VideoPlayerContext::~VideoPlayerContext()
{
	audioStream.Reset();
	audioStream = {};

    state.exited = true;
    condition.notify_all();

    videoThreadPool->RemoveTasks();
    audioThreadPool->RemoveTasks();

    videoThreadPool->Join();
    audioThreadPool->Join();

    demuxerThread.Reset();
}

Picture VideoPlayerContext::GetPicture()
{
    if (picture)
    {
		return picture;
    }
	pictures.try_dequeue(picture);
    if (!picture)
    {
		condition.notify_one();
    }

	return picture;
}

Picture VideoPlayerContext::GetAudioFrame()
{
    if (audioFrame)
    {
		return audioFrame;
    }

    audioFrames.try_dequeue(audioFrame);
	return audioFrame;
}

void VideoPlayerContext::PopPicture()
{
	pictureSize--;
	picture = {};
    condition.notify_one();
}

void VideoPlayerContext::PopAudioFrame()
{
	audioFrame = {};
}

uint32_t VideoPlayerContext::GetAudioData(uint8_t *data, uint32_t samples)
{
	if (pause)
	{
		return 0;
	}

	uint32_t numSamples = 0;
	//if (audioMutex.try_lock())
	{
		numSamples = WriteAudioData(data, samples);
		while (numSamples != samples)
		{
			outputAudioFrame = GetAudioFrame();
            if (!outputAudioFrame)
            {
				break;
            }

			PopAudioFrame();
			unconsumedSamples = outputAudioFrame.GetWidth();
			numSamples += WriteAudioData(&data[numSamples * sizeof(float) * 2], samples - numSamples);
		}
		//audioMutex.unlock();
	}

	return numSamples;
}

uint32_t VideoPlayerContext::WriteAudioData(void *data, uint32_t samples)
{
	uint32_t numSamples = 0;
	if (unconsumedSamples > 0)
	{
		uint32_t request = std::min(unconsumedSamples, samples);

		size_t sizePerSample = sizeof(float) * 2;
		auto size = request * sizePerSample;
		memcpy(data, outputAudioFrame.GetData() + (outputAudioFrame.GetWidth() - unconsumedSamples) * sizePerSample, size);

		numSamples += request;
		unconsumedSamples -= request;
	}

	return numSamples;
}

VideoPlayerComponent::VideoPlayerComponent() :
    player{}
{

}

VideoPlayerComponent::VideoPlayerComponent(const String &path, int cacheSize, const Vision::DecodingPreference &preference, const VideoDecodeCallbacks &callbacks) :
    player{}
{
	Ref<Demuxer>    demuxer         = new Vision::FFDemuxer;
	Ref<VideoCodec> decoder         = new Vision::FFCodec;
	Ref<VideoCodec> audioDecoder    = new Vision::FFCodec;
	Ref<VideoCodec> subtitleDecoder = new Vision::FFCodec;
	decoder.InterpretAs<Vision::FFCodec>()->SetPreference(preference);
	if (demuxer->Open(path, decoder, audioDecoder, subtitleDecoder) != CodecError::Success)
    {
		return;
    }

    player = { new VideoPlayerContext{demuxer, decoder, audioDecoder, subtitleDecoder, cacheSize, callbacks} };
}

VideoPlayerComponent::VideoPlayerComponent(Ref<Demuxer> demuxer, Ref<VideoCodec> decoder, Ref<VideoCodec> audioDecoder, Ref<VideoCodec> subtitleDecoder) :
    player{new VideoPlayerContext{demuxer, decoder, audioDecoder, subtitleDecoder}}
{

}

VideoPlayerComponent::~VideoPlayerComponent()
{
    player.Reset();
}

void VideoPlayerComponent::StartPlay()
{
	player->StartPlay();
}

static inline int64_t RescaleTimestamp(int64_t timestamp, int64_t a, int64_t b)
{
	return (timestamp * a + b / 2) / b;
}

static inline int CompareTimestamp(int64_t timestampA, Rational timebaseA, int64_t timestampB, Rational timebaseB)
{
	int64_t a = timebaseA.numerator * (int64_t) timebaseB.denominator;
	int64_t b = timebaseB.numerator * (int64_t) timebaseA.denominator;

    if (std::abs(a) | a | std::abs(b) <= std::numeric_limits<int32_t>::max())
    {
		return (timestampA * a > timestampB * b) - (timestampA * a < timestampB * b);
    }
	if (RescaleTimestamp(timestampA, a, b) < timestampB)
    {
		return -1;
    }
	if (RescaleTimestamp(timestampB, b, a) < timestampA)
    {
		return 1;
    }

	return 0;
}

Picture VideoPlayerComponent::GetLivePicture()
{
	Animator *animator = GetAnimator();

    Picture picture = GetPicture();
    if (picture)
    {
		int64_t videoTimestamp = picture.GetTimestamp();
		Rational videoTimebase  = picture.GetTimebase();

		float deltaTime = Time::DeltaTime;
        double diff = 0;

		double videoSeconds = 0.0f;
        double audioSeconds = 0.0f;
        videoSeconds = videoTimestamp * videoTimebase.Normalize();

        Picture audioFrame = player->GetCurrentAudioFrame();
        if (audioFrame)
        {
			auto unconsumedSamples = player->GetUnconsumedSamples();
			int64_t audioTimestamp = audioFrame.GetTimestamp() - (audioFrame.GetWidth() - unconsumedSamples);
			Rational audioTimebase = audioFrame.GetTimebase();
			audioSeconds = audioTimestamp * audioTimebase.Normalize();
            videoSeconds = videoSeconds + animator->Accumulator + deltaTime;
			diff = (audioSeconds - videoSeconds);
        }

        if (std::abs(diff) > 0.5)
		{
            if (diff > 0)
            {
				deltaTime = 0;
            }
			else if (diff < 0)
            {
				deltaTime += std::abs(diff);
            }
        }

		if (animator->TryMoveToNextFrame(deltaTime))
		{
			PopPicture();
		}
    }

    if (currentPicture != picture)
    {
		currentPicture = picture;
    }
    else
    {
		picture = {};
    }

    return picture;
}

Picture VideoPlayerComponent::GetPicture()
{
    return player->GetPicture();
}

Picture VideoPlayerComponent::GetAudioFrame()
{
    return player->GetAudioFrame();
}

void VideoPlayerComponent::PopPicture()
{
    player->PopPicture();
}

void VideoPlayerComponent::PopAudioFrame()
{
	Animator *animator = GetAnimator();
	animator->Accumulator = 0;
    player->PopAudioFrame();
}

void VideoPlayerComponent::Seek(double seconds, int64_t min, int64_t max)
{
    player->Seek(seconds, min, max);
}

bool VideoPlayerComponent::IsEof() const
{
	return player->IsEof();
}

void VideoPlayerComponent::Swap(VideoPlayerComponent &other)
{
    player.Swap(other.player);
}

Animator *VideoPlayerComponent::GetAnimator() const
{
    return player->decoder->GetAddress<Animator>();
}

const String &VideoPlayerComponent::GetSource() const
{
    return player->GetSource();
}

const Vision::DisplayOrientation *VideoPlayerComponent::GetDisplayOrientation() const
{
	return player->GetDisplayOrientation();
}

void VideoPlayerComponent::OnPause(bool enabled)
{
	player->SetPause(enabled);
}

CodecError VideoPlayerComponent::SwitchTrack(MediaType mediaType, int index)
{
	return player->SwitchTrack(mediaType, index);
}

void VideoPlayerComponent::EnumerateTracks(MediaType mediaType, std::vector<Vision::TrackInfo> &tracks)
{
	player->EnumerateTracks(mediaType, tracks);
}

bool VideoPlayerComponent::HasStream(MediaType type) const
{
    if (type == MediaType::Video)
    {
		return player->decoder;
    }
    else if (type == MediaType::Audio)
    {
		return player->audioDecoder;
    }

    return false;
}

bool VideoPlayerComponent::operator!()
{
	return !player;
}

}
