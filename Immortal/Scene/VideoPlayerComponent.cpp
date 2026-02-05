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

struct VideoPlayerContext : public IClass
{
public:
	VideoPlayerContext(int cacheSize, VideoPlayerMode mode);

    ~VideoPlayerContext();

	CodecError Open(const String &path, int cacheSize = 3, const Vision::DecodingPreference &preference = Vision::DecodingPreference::Auto, VideoPlayerMode mode = VideoPlayerMode::Playing, StreamEnabledFlags flags = StreamEnabledFlags::None);

	CodecError Open(const String &path, const Vision::DecodingPreference &preference, StreamEnabledFlags flags);

    VideoPlayerContext(const VideoPlayerContext &&other) = delete;

	void Playback();

	void Transcode();

    VideoPlayerContext &operator=(const VideoPlayerContext &&other) = delete;

    void Seek(MediaType type, int64_t pts, int64_t min, int64_t max);

    Picture GetPicture();

    Picture GetAudioFrame();

    void PopPicture();

    void PopAudioFrame();

    void StartPlay();

	void SetAudioOutputSpec(const Vision::AudioFormatSpec &outputSpec);

    void CreateAudioStream();

    uint32_t GetAudioData(uint8_t *data, uint32_t samples);

	uint32_t WriteAudioData(void *data, uint32_t samples);

    void EndOfFile(Vision::Interface::Codec *decoder, const std::function<void(Picture &&)> &callback, MediaType type);

	void GetPictures(bool eof);

	void Join();

	Picture ResampleAudioFrame(Picture &picture);

	void GetVideoPictures(CodedFrame &&codedFrame);

	void GetAudioPictures(CodedFrame &&codedFrame);

public:
    const Vision::DisplayOrientation *GetDisplayOrientation() const
    {
		return decoder->GetProperty<Vision::DisplayOrientation>();
    }

	CodecError GetStreamInfo(MediaType type, CodecInfo &streamInfo)
	{
		return demuxer ? demuxer->GetStreamInfo(type, streamInfo) : CodecError::ExternalFailed;
	}

    void ClearPictures(ConcurrentQueue<Picture> &pictures, Picture &picture, int &size)
    {
		ConcurrentQueue<Picture> empty;
		pictures.swap(empty);
		picture = {};
		size = 0;
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
		demuxer.InterpretAs<Vision::FFFormat>()->EnumerateTracks(mediaType, tracks);
    }

    CodecError SwitchTrack(MediaType mediaType, int index)
	{
		return demuxer.InterpretAs<Vision::FFFormat>()->SwitchTrack(mediaType, index);
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

    void SetFilterGraph(const std::shared_ptr<FilterGraphComponent> &graph, Format _format)
	{
        if (!asyncComputeThread)
        {
			asyncComputeThread = new AsyncComputeThread{Graphics::GetDevice()};
			queue = Graphics::GetDevice()->CreateQueue(QueueType::Compute);
			asyncComputeThread->Execute<SetQueueTask>(queue);
        }
		filterGraph = graph;
		format = _format;
	}

	void SetCallbacks(const VideoDecodeCallbacks &_callbacks)
	{
		callbacks = _callbacks;
	}

public:
	VideoPlayerMode mode;

    Thread demuxerThread;

    std::atomic_bool decoding = false;

    ConcurrentQueue<CodedFrame> codedFrames;

    ConcurrentQueue<void *> memory;

    std::unique_ptr<ThreadPool> videoThreadPool;

    std::unique_ptr<ThreadPool> audioThreadPool;

    std::unique_ptr<ThreadPool> subtitleThreadPool;

    AudioStream *audioStream;

    Vision::SampleConverter sampleConverter;

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

    Ref<Vision::MediaFormat> demuxer;

    ConcurrentQueue<Picture> pictures;

    ConcurrentQueue<Picture> audioFrames;

    ConcurrentQueue<Picture> subtitles;

    Picture picture;

    Picture audioFrame;

    Picture outputAudioFrame;

    int kCacheSize;

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

    int pictureSize = 0;

    VideoDecodeCallbacks callbacks{};

    bool pause = false;

    double externalClock = -1.0f;

    int64_t lastAudioTimestamp = 0;

    bool audioDeviceChanged = false;

    std::function<void()> task;

    ColorSpace colorSpace = ColorSpace::Unspecified;

    ColorTransferCharacteristic colorTransferCharacteristic = ColorTransferCharacteristic::Unspecified;

    std::shared_ptr<FilterGraphComponent> filterGraph;

	Format format;

    URef<AsyncComputeThread> asyncComputeThread;

    Ref<Queue> queue;

    std::atomic<int> asyncComputeTaskCount = 0;

	Ref<ScaleFilter> scaleFilter;

	std::atomic_bool done = false;

#if IMMORTAL_HAVE_VIDEO_PLAYER_STATISTIC
	Timer timer;
#endif
};

CodecError AsyncDecode(const Vision::CodedFrame &codedFrame, Vision::Interface::Codec *decoder)
{
    return decoder->Decode(codedFrame);
}

void VideoPlayerContext::GetPictures(bool eof = false)
{
    Vision::Picture picture;
	while (decoder->GetPicture(picture) == CodecError::Success)
	{
		if (filterGraph)
		{
			static bool hasScale = false;

			if (format.IsType(Format::YUV) && !scaleFilter)
			{
				auto &format = picture.GetFormat();
				Format dstFormat = Format::RGBA8;
				bool isHighBitDepth = format.IsType(Format::_10Bits) || format.IsType(Format::_12Bits);
				dstFormat = isHighBitDepth ? Format::R16G16B16A16_UNORM : Format::RGBA8;
				scaleFilter = filterGraph->Insert<ScaleFilter>(0, picture.GetFormat(), dstFormat, picture.GetWidth(), picture.GetHeight());
			}

			asyncComputeTaskCount++;
			asyncComputeThread->Execute<AsyncTask>(AsyncTaskType::BeginRecording);
			filterGraph->Execute({picture}, asyncComputeThread);

			SamplingFactor factors[SamplingFactor::kMaxSublayer] = {};
			GetSamplingFactor(format, factors);
			auto &output = filterGraph->QueryOutputs();
			picture = Picture{output[0]->GetWidth() << factors[0].x, output[0]->GetHeight() << factors[0].y, format, true};
			Graphics::Transfer(picture, output, asyncComputeThread);
			asyncComputeThread->Execute<AsyncTask>(AsyncTaskType::EndRecording);
			asyncComputeThread->Execute<ExecutionCompletedTask>([=, this]() {
				callbacks.VideoDecodeFinishSlot(std::move((Picture &&)picture));
				asyncComputeTaskCount--;
				if (eof && asyncComputeTaskCount == 0)
				{
					done = true;
					done.notify_one();
				}
				condition.notify_one();
			});
			asyncComputeThread->Execute<AsyncTask>(AsyncTaskType::Submiting);
		}
        else
        {
			callbacks.VideoDecodeFinishSlot(std::move(picture));
        }
#if IMMORTAL_HAVE_VIDEO_PLAYER_STATISTIC
		frames++;
#endif
	}
}

void VideoPlayerContext::EndOfFile(Vision::Interface::Codec *decoder, const std::function<void(Picture &&)> &callback, MediaType type)
{
	CodedFrame codedFrame{(uint8_t *)nullptr};
	decoder->Decode(codedFrame);

	if (type == MediaType::Video)
	{
		GetPictures(true);
	}
	else
	{
		while (decoder->GetPicture(picture) == CodecError::Success)
		{
			callback(std::move(ResampleAudioFrame(picture)));
			audioSize++;
		}

		if (sampleConverter)
		{
			Picture picture = sampleConverter.GetRemainingSamples();
			if (picture)
			{
				callback(std::move(picture));
				audioSize++;
			}
		}
	}

	Picture picture = Picture{0, 0, Format::None};
	picture.SetFlags(Vision::PictureFlags::Eof);
    if (filterGraph && type == MediaType::Video)
	{
		done.wait(false);
    }

	callback(std::move(picture));
}

void VideoPlayerContext::StartPlay()
{
	demuxerThread.Start(std::move(task));
	timer.Start();

	auto stem = demuxer->GetSource().GetStem();
	demuxerThread.SetDescription(std::string("Demux::") + stem);
	if (videoThreadPool)
	{
		videoThreadPool->SetDebugDescription(0, std::string("VideoDecode::") + stem);
	}
	if (audioThreadPool)
	{
		audioThreadPool->SetDebugDescription(0, std::string("AudioDecode::") + stem);
	}
}

void VideoPlayerContext::SetAudioOutputSpec(const Vision::AudioFormatSpec &outputSpec)
{
	auto decoder = audioDecoder.InterpretAs<Vision::FFCodec>();
	Vision::AudioFormatSpec inputSpec = decoder->GetAudioFormat();
	if (inputSpec.layout != outputSpec.layout || inputSpec.format != outputSpec.format)
	{
		if (!sampleConverter.SetOptions(outputSpec, inputSpec))
		{
			CLOG_ERROR("Error when creating sampleConverter. Audio playing maybe corrupted!");
		}
	}
}

Picture VideoPlayerContext::ResampleAudioFrame(Picture &picture)
{
	if (sampleConverter)
	{
		auto &outputAudioFormat = sampleConverter.GetOutputFormat();
		int rescaledSampleWidth = sampleConverter.RescaleRound(picture.GetWidth());
		Picture rescaledPicture = {(uint32_t) rescaledSampleWidth * outputAudioFormat.numChannel, 1, outputAudioFormat.format, true};
		rescaledPicture.SetWidth(rescaledSampleWidth);
		if (sampleConverter.Convert(rescaledPicture, picture))
		{
			rescaledPicture.SetSampleRate(outputAudioFormat.sampleRate);
			rescaledPicture.SetTimestamp(picture.GetTimestamp());
			return rescaledPicture;
		}
	}

	return picture;
}

void VideoPlayerContext::CreateAudioStream()
{
	AudioDevice *audioDevice = AudioDevice::GetInstance();

	auto audioFormat = audioDevice->GetFormat();
	Vision::AudioFormatSpec outputSpec{
        .format     = audioFormat.format,
		.layout     = Vision::GetLayoutFromMask(audioFormat.mask),
        .sampleRate = audioFormat.sampleRate,
		.numChannel = audioFormat.channels,
    };

	SetAudioOutputSpec(outputSpec);

	audioStream = audioDevice->CreateAudioStream([=, this](void *data, uint32_t samples) -> uint32_t {
		return GetAudioData((uint8_t *)data, samples);
	});

	audioStream->SetDebugName(demuxer->GetSource().GetString());
}

void VideoPlayerContext::Join()
{

}

void VideoPlayerContext::GetVideoPictures(CodedFrame &&codedFrame)
{
	if (!decoder)
	{
		return;
	}

	videoThreadPool->Enqueue([=, this]() -> void {
		AsyncDecode(codedFrame, decoder);
		Picture picture{};
		while (decoder->GetPicture(picture) == CodecError::Success)
		{
#if IMMORTAL_HAVE_VIDEO_PLAYER_STATISTIC
			frames++;
#endif
			if (colorSpace != ColorSpace::Unspecified)
			{
				picture.SetColorSpace(colorSpace);
			}
			if (colorTransferCharacteristic != ColorTransferCharacteristic::Unspecified)
			{
				picture.SetColorTransferCharacteristic(colorTransferCharacteristic);
			}
			pictures.enqueue(picture);
			pictureSize++;
		}
	});
}

void VideoPlayerContext::GetAudioPictures(CodedFrame &&codedFrame)
{
	if (!audioDecoder)
	{
		return;
	}

	audioThreadPool->Enqueue([=, this]() -> void {
		if (codedFrame)
		{
			AsyncDecode(codedFrame, audioDecoder);
		}
		Picture picture{};
		while (audioDecoder->GetPicture(picture) == CodecError::Success)
		{
			if (audioDeviceChanged)
			{
				ClearPictures(audioFrames, audioFrame, audioSize);
				auto audioDevice = AudioDevice::GetInstance();
				audioStream->Stop();
				audioStream->Reset();
				audioDevice->DestroyAudioStream(&audioStream);
				CreateAudioStream();
				audioDeviceChanged = false;
			}
			audioFrames.enqueue(ResampleAudioFrame(picture));
#if IMMORTAL_HAVE_VIDEO_PLAYER_STATISTIC
			audioSize++;
#endif
		}
	});
}

VideoPlayerContext::VideoPlayerContext(int cacheSize, VideoPlayerMode mode) :
    ICLASS,
    mode{mode},
    demuxerThread{},
    videoThreadPool{},
    audioThreadPool{},
    subtitleThreadPool{},
    decoder{},
    audioDecoder{},
    subtitleDecoder{},
    demuxer{},
    state{},
    kCacheSize{cacheSize},
    callbacks{},
    timer{},
    audioStream{}
{
}

CodecError VideoPlayerContext::Open(const String &path, int cacheSize, const Vision::DecodingPreference &preference, VideoPlayerMode mode, StreamEnabledFlags flags)
{
	auto ret = Open(path, preference, flags);
	if (ret != CodecError::Success)
	{
		CLOG_ERROR("Error when opening {}", path);
		return ret;
	}

	if (mode == VideoPlayerMode::Playing)
	{
		Playback();
	}
	else if (mode == VideoPlayerMode::Transcoding)
	{
		Transcode();
	}

	return CodecError::Success;
}

CodecError VideoPlayerContext::Open(const String &path, const Vision::DecodingPreference &preference, StreamEnabledFlags flags)
{
	demuxer = new Vision::FFFormat{};
	if (demuxer->Open(path) != CodecError::Success)
	{
		demuxer = {};
		return CodecError::ExternalFailed;
	}

	if (mode == VideoPlayerMode::MetaReading)
	{
		return CodecError::Success;
	}

	Ref<VideoCodec> *codecs[] = {&decoder, &audioDecoder, &subtitleDecoder};
	std::unique_ptr<ThreadPool> *threadPools[] = {&videoThreadPool, &audioThreadPool, &subtitleThreadPool};
	for (int i = 0; i < SL_ARRAY_LENGTH(codecs); i++)
	{
		if (flags != StreamEnabledFlags::None && !(flags & StreamEnabledFlags(BIT(i))))
		{
			continue;
		}

		CodecInfo info{};
		if (demuxer->GetStreamInfo(MediaType(i), info) == CodecError::Success)
		{
			auto decoder = new Vision::FFCodec{};
			decoder->SetPreference(preference);
			if (decoder->OpenDecoder(info) != CodecError::Success)
			{
				return CodecError::FailedToCallDecoder;
			}

			auto animator = decoder->GetAddress<Animator>();
			*animator = demuxer->GetAnimator(MediaType(i));

			*codecs[i] = decoder;
			threadPools[i]->reset(new ThreadPool{1});
		}
	}

	return CodecError::Success;
}

void VideoPlayerContext::Playback()
{
	if (audioDecoder)
	{
		AudioDevice *audioDevice = AudioDevice::GetInstance();
		if (audioDevice)
		{
			CreateAudioStream();
            audioDevice->SetOnEvent([=, this] (Event &event) {
                if (event.GetType() == Event::Type::AudioDefaultDeviceChanged)
				{
					audioDeviceChanged = true;
                }
            });
		}
	}

	ThreadPool *primaryThread = videoThreadPool ? videoThreadPool.get() : audioThreadPool.get();
	int *primarySize = videoThreadPool ? &pictureSize : &audioSize;
	if (audioDecoder && decoder)
	{
		CodecInfo info{};
		if (demuxer->GetStreamInfo(MediaType::Video, info) == CodecError::Success)
		{
			// Workaround
			if (info.width < 1920 && info.height < 1080)
			{
				kCacheSize = 1;
				primaryThread = audioThreadPool.get();
				primarySize = &audioSize;
			}
		}
	}

    task = [=, this]() {
    while (true)
    {
        std::unique_lock lock{ mutex.demux };
        condition.wait(lock, [=, this] {
			return state.exited || (!eof &&
                ((*primarySize + primaryThread->TaskSize()) <= kCacheSize));
        });

        if (state.exited)
        {
			double time = timer.Duration();
			CLOG_INFO("Decoding Statistic: frames:{}, time:{}, fps:{}", frames, time, frames / time);
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
					if (decoder)
					{
						GetVideoPictures(std::move(codedFrame));
					}
					if (audioDecoder)
					{
						GetAudioPictures(std::move(codedFrame));
					}
				}
				eof = true;
            }
			continue;
		}

        switch (codedFrame.GetType())
        {
		case MediaType::Video:
		{
			GetVideoPictures(std::move(codedFrame));
			break;
        }
		case MediaType::Audio:
        {
			GetAudioPictures(std::move(codedFrame));
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
    };
	StartPlay();
}

void VideoPlayerContext::Transcode()
{
	ThreadPool *primaryThread = videoThreadPool ? videoThreadPool.get() : audioThreadPool.get();
	if (primaryThread)
	{
		primaryThread->OnNotify([=, this] {
			condition.notify_one();
		});
	}

    task = [=, this]() {
    while (true)
    {
        std::unique_lock lock{ mutex.demux };
        condition.wait(lock, [=, this] {
			bool hasTask = asyncComputeTaskCount + primaryThread->TaskSize() <= kCacheSize;
			return state.exited || eof || hasTask;
        });

        if (state.exited)
        {
			double time = timer.Duration();
			CLOG_INFO("Decoding Statistic: frames:{}, time:{}, fps:{}", frames, time, frames / time);
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
					if (decoder)
					{
						videoThreadPool->Enqueue([=, this] {
							EndOfFile(decoder, callbacks.VideoDecodeFinishSlot, MediaType::Video);
						});
					}

					if (audioDecoder)
					{
						audioThreadPool->Enqueue([=, this]() -> void {
							EndOfFile(audioDecoder, callbacks.AudioDecodeFinishSlot, MediaType::Audio);
						});
					}
                }
				eof = true;
            }
			continue;
		}

        switch (codedFrame.GetType())
        {
		case MediaType::Video:
        {
			if (decoder)
			{
				videoThreadPool->Enqueue([=, this]() -> void {
					AsyncDecode(codedFrame, decoder);
					GetPictures();
				});
			}
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
						callbacks.AudioDecodeFinishSlot(std::move(ResampleAudioFrame(picture)));
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
    }};
}

void VideoPlayerContext::Seek(MediaType type, int64_t pts, int64_t min, int64_t max)
{
	std::unique_lock lock{mutex.demux};
	if (videoThreadPool)
	{
		videoThreadPool->RemoveTasks();
	}

	if (audioThreadPool)
	{
		if (audioStream)
		{
			audioStream->Stop();
			audioStream->Reset();
		}
		audioThreadPool->RemoveTasks();
	}

	if (videoThreadPool)
	{
		videoThreadPool->Join();
		if (eof)
		{
			decoder->Flush();
		}
		ClearPictures(pictures, picture, pictureSize);
	}

	if (audioThreadPool)
	{
		audioThreadPool->Join();
		if (eof)
		{
			audioDecoder->Flush();
		}
		ClearPictures(audioFrames, audioFrame, audioSize);
	}

	eof = false;
	demuxer->Seek(type, pts, min, max);
	condition.notify_all();

	if (audioStream)
	{
		audioStream->Start();
	}
}

VideoPlayerContext::~VideoPlayerContext()
{
	if (audioStream)
	{
		AudioDevice *audioDevice = AudioDevice::GetInstance();
		audioDevice->DestroyAudioStream(&audioStream);
	}

    state.exited = true;
    condition.notify_all();

    demuxerThread = {};

	if (videoThreadPool)
	{
		videoThreadPool->RemoveTasks();
	}
	if (audioThreadPool)
	{
		audioThreadPool->RemoveTasks();
	}

	if (videoThreadPool)
	{
		videoThreadPool->Join();
	}
	if (audioThreadPool)
	{
		audioThreadPool->Join();
	}

    if (filterGraph)
    {
		filterGraph.reset();
		asyncComputeThread.Reset();
		queue.Reset();
    }
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
	if (!videoThreadPool && !audioFrame)
	{
		condition.notify_one();
	}

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
	lastAudioTimestamp = audioFrame.GetTimestamp();
	audioFrame = {};
	audioSize--;
}

uint32_t VideoPlayerContext::GetAudioData(uint8_t *data, uint32_t samples)
{
	if (pause)
	{
		return 0;
	}

	uint32_t numSamples = WriteAudioData(data, samples);
	while (numSamples != samples)
	{
		outputAudioFrame = GetAudioFrame();
        if (!outputAudioFrame)
        {
			break;
        }

		PopAudioFrame();
		unconsumedSamples = outputAudioFrame.GetWidth();

        int numChannel = 2;
		if (sampleConverter)
		{
			numChannel = sampleConverter.GetOutputFormat().numChannel;
		}
		size_t bytePerPixel = outputAudioFrame.GetFormat().GetTexelSize() * numChannel;
		numSamples += WriteAudioData(&data[numSamples * bytePerPixel], samples - numSamples);
	}

	return numSamples;
}

uint32_t VideoPlayerContext::WriteAudioData(void *data, uint32_t samples)
{
	uint32_t numSamples = 0;
	if (unconsumedSamples > 0)
	{
		uint32_t request = std::min(unconsumedSamples, samples);

        int numChannel = 2;
        if (sampleConverter)
        {
			numChannel = sampleConverter.GetOutputFormat().numChannel;
        }

		size_t bytePerPixel = outputAudioFrame.GetFormat().GetTexelSize() * numChannel;

		auto size = request * bytePerPixel;
        auto consumed = (outputAudioFrame.GetWidth() - unconsumedSamples) * bytePerPixel;

		memcpy(data, outputAudioFrame.GetData() + consumed, size);

		numSamples += request;
		unconsumedSamples -= request;
	}

	return numSamples;
}

VideoPlayerComponent::VideoPlayerComponent() :
    player{}
{

}

VideoPlayerComponent::VideoPlayerComponent(const String &path, int cacheSize, const Vision::DecodingPreference &preference, VideoPlayerMode mode, StreamEnabledFlags flags) :
    player{}
{
	player = new VideoPlayerContext{cacheSize, mode};
	if (player->Open(path, cacheSize, preference, mode, flags) != CodecError::Success)
	{
		player = {};
	}
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
	Animator *animator = GetAnimator(MediaType::Video);

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

  //      if (std::abs(diff) > 0.5)
		//{
  //          if (diff > 0)
  //          {
		//		deltaTime = 0;
  //          }
		//	else if (diff < 0)
  //          {
		//		deltaTime += std::abs(diff);
  //          }
  //      }

		if (animator->TryMoveToNextFrame(deltaTime, speed))
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
	Animator *animator = GetAnimator(MediaType::Video);
	animator->Accumulator = 0;
    player->PopAudioFrame();
}

void VideoPlayerComponent::Seek(MediaType type, int64_t pts, int64_t min, int64_t max)
{
    player->Seek(type, pts, min, max);
}

bool VideoPlayerComponent::IsEof() const
{
	return player->IsEof();
}

void VideoPlayerComponent::Swap(VideoPlayerComponent &other)
{
    player.Swap(other.player);
}

Animator *VideoPlayerComponent::GetAnimator(MediaType type) const
{
	return &player->demuxer->GetAnimator(type);
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
	CodecInfo info{};
    return player->demuxer->GetStreamInfo(type, info) == CodecError::Success;
}

CodecError VideoPlayerComponent::GetStreamInfo(MediaType type, CodecInfo &streamInfo)
{
	return player->GetStreamInfo(type, streamInfo);
}

void VideoPlayerComponent::SetFilterGraph(const std::shared_ptr<FilterGraphComponent> &graph, Format format)
{
	player->SetFilterGraph(graph, format);
}

void VideoPlayerComponent::SetCallbacks(const VideoDecodeCallbacks &callbacks)
{
	player->SetCallbacks(callbacks);
}

void VideoPlayerComponent::SetAudioOutputSpec(const Vision::AudioFormatSpec &outputSpec)
{
	player->SetAudioOutputSpec(outputSpec);
}

void VideoPlayerComponent::Join()
{
	player->Join();
}

bool VideoPlayerComponent::operator!()
{
	return !player;
}

int64_t VideoPlayerComponent::GetLastAudioTimestamp() const
{
	return player->lastAudioTimestamp;
}

Rational VideoPlayerComponent::GetAudioTimebase() const
{
	return player->audioDecoder.InterpretAs<Vision::FFCodec>()->GetTimebase();
}

VideoPlayerComponent::operator bool() const
{
	return !!player;
}

void VideoPlayerComponent::SetSpeed(double value)
{
	speed = value;
}

}
