/**
 * Copyright (C) 2022-2024, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#include "VideoPlayerComponent.h"
#include "FfplayVideoState.h"
#include "Audio/Device.h"
#include "Config.h"
#include "Vision/MediaFormat/FFFormat.h"
#include "Vision/Video/FFCodec.h"

#include <shared_mutex>
#include <cmath>
#include <algorithm>
#include <chrono>
#include <cstring>
#include <deque>

#define IMMORTAL_HAVE_VIDEO_PLAYER_STATISTIC 1

namespace Immortal
{

static constexpr int    MIN_FRAMES                 = 25;
static constexpr size_t MAX_QUEUE_SIZE             = 15 * 1024 * 1024;
static constexpr double AV_SYNC_THRESHOLD_MIN      = 0.04;
static constexpr double AV_SYNC_THRESHOLD_MAX      = 0.1;
static constexpr double AV_SYNC_FRAMEDUP_THRESHOLD = 0.1;
static constexpr int    AUDIO_DIFF_AVG_NB          = 20;
static constexpr double AV_NOSYNC_THRESHOLD        = 10.0;
static constexpr size_t TIMELINE_STEP_CACHE_LIMIT  = 16;
static constexpr size_t TIMELINE_STEP_PREFETCH     = 4;
static constexpr int64_t TIMELINE_STEP_NEAR_FRAMES = 8;

namespace
{

static double SteadyTimeSeconds()
{
	using namespace std::chrono;
	return duration<double>(steady_clock::now().time_since_epoch()).count();
}

static double GetClock(const FfplayClock &c)
{
	if (c.paused)
		return c.pts;
	double time = SteadyTimeSeconds();
	return c.ptsDrift + time - (time - c.lastUpdated) * (1.0 - c.speed);
}

static void SetClockAt(FfplayClock &c, double pts, int serial, double time)
{
	c.pts         = pts;
	c.lastUpdated = time;
	c.ptsDrift    = pts - time;
	c.serial      = serial;
}

static void ApplyAudioVolume(void *buffer, uint32_t samples, int numChannel, Format format, float volume)
{
    if (!buffer || samples == 0 || numChannel <= 0 || volume == 1.0f)
    {
        return;
    }

    const size_t count = (size_t)samples * (size_t)std::max(1, numChannel);
    switch (Format::ValueType(format))
    {
    case Format::R8_UINT:
    case Format::R8_UINTP:
    {
        auto *values = static_cast<uint8_t *>(buffer);
        for (size_t i = 0; i < count; i++)
        {
            int sample = (int)std::lround(((int)values[i] - 128) * volume + 128.0f);
            values[i] = (uint8_t)std::clamp(sample, 0, 255);
        }
        break;
    }
    case Format::R16_SINT:
    case Format::R16_SINTP:
    {
        auto *values = static_cast<int16_t *>(buffer);
        for (size_t i = 0; i < count; i++)
        {
            int sample = (int)std::lround((double)values[i] * (double)volume);
            values[i] = (int16_t)std::clamp(sample, (int)std::numeric_limits<int16_t>::min(), (int)std::numeric_limits<int16_t>::max());
        }
        break;
    }
    case Format::R32_SINT:
    case Format::R32_SINTP:
    {
        auto *values = static_cast<int32_t *>(buffer);
        for (size_t i = 0; i < count; i++)
        {
            long long sample = (long long)std::llround((double)values[i] * (double)volume);
            values[i] = (int32_t)std::clamp(sample,
                                            (long long)std::numeric_limits<int32_t>::min(),
                                            (long long)std::numeric_limits<int32_t>::max());
        }
        break;
    }
    case Format::FLOAT:
    case Format::FLOATP:
    {
        auto *values = static_cast<float *>(buffer);
        for (size_t i = 0; i < count; i++)
        {
            values[i] *= volume;
        }
        break;
    }
    case Format::DOUBLE:
    case Format::DOUBLEP:
    {
        auto *values = static_cast<double *>(buffer);
        for (size_t i = 0; i < count; i++)
        {
            values[i] *= (double)volume;
        }
        break;
    }
    default:
        break;
    }
}

static void SetClock(FfplayClock &c, double pts, int serial)
{
	SetClockAt(c, pts, serial, SteadyTimeSeconds());
}

static void InitClock(FfplayClock &c, std::atomic<int> *queueSerial)
{
	c.speed       = 1.0;
	c.paused      = false;
	c.queueSerial = queueSerial;
	SetClock(c, NAN, -1);
}

static void SyncClockToSlave(FfplayClock &c, const FfplayClock &slave)
{
	double clockVal  = GetClock(c);
	double slaveVal  = GetClock(slave);
	if (!std::isnan(slaveVal) && (std::isnan(clockVal) || std::fabs(clockVal - slaveVal) > AV_NOSYNC_THRESHOLD))
	{
		SetClock(c, slaveVal, slave.serial);
	}
}

static double ComputeTargetDelay(double delay, double diff, FfplayVideoState &vs)
{
	double syncThreshold;

	if (vs.avSyncType == FfplayAvSyncType::VideoMaster)
		return delay;

	if (!std::isnan(diff))
	{
		if (diff <= -AV_NOSYNC_THRESHOLD)
			return 0;

		if (diff >= AV_NOSYNC_THRESHOLD)
			return 2.0 * delay;

		syncThreshold = std::max(AV_SYNC_THRESHOLD_MIN, std::min(AV_SYNC_THRESHOLD_MAX, delay));

		if (std::fabs(diff) < AV_NOSYNC_THRESHOLD)
		{
			if (diff <= -syncThreshold)
				delay = std::max(0.0, delay + diff);
			else if (diff >= syncThreshold && delay > AV_SYNC_FRAMEDUP_THRESHOLD)
				delay = delay + diff;
			else if (diff >= syncThreshold)
				delay = 2.0 * delay;
		}
	}
	return delay;
}

static double VpDuration(double lastPts, double curPts, double maxFrameDuration, double fallbackDur)
{
	double dur = curPts - lastPts;
	if (std::isnan(dur) || dur <= 0 || dur > maxFrameDuration)
		return fallbackDur;
	return dur;
}

static bool HasComparablePts(const Picture &picture)
{
	return picture && picture.GetTimestamp() != std::numeric_limits<int64_t>::min() &&
	       picture.GetTimebase().denominator != 0;
}

static long double PtsSeconds(int64_t pts, Rational tb)
{
	if (tb.denominator == 0)
		return 0.0L;
	return (long double)pts * (long double)tb.numerator / (long double)tb.denominator;
}

static int ComparePicturePts(const Picture &picture, int64_t targetPts, Rational targetTb)
{
	if (!HasComparablePts(picture) || targetPts == std::numeric_limits<int64_t>::min() ||
	    targetTb.denominator == 0)
	{
		return 0;
	}

	if (picture.GetTimebase().numerator == targetTb.numerator &&
	    picture.GetTimebase().denominator == targetTb.denominator)
	{
		if (picture.GetTimestamp() < targetPts)
			return -1;
		if (picture.GetTimestamp() > targetPts)
			return 1;
		return 0;
	}

	const long double a = PtsSeconds(picture.GetTimestamp(), picture.GetTimebase());
	const long double b = PtsSeconds(targetPts, targetTb);
	const long double eps = 1.0e-9L;
	if (a + eps < b)
		return -1;
	if (a > b + eps)
		return 1;
	return 0;
}

enum class TimelineStepCacheSide
{
	Backward,
	Forward
};

static std::deque<Picture> &TimelineStepCache(VideoPlayerComponent *component, TimelineStepCacheSide side)
{
	return side == TimelineStepCacheSide::Forward
		? component->timelineFrameWindow.forward
		: component->timelineFrameWindow.backward;
}

static std::deque<Picture> &OppositeTimelineStepCache(VideoPlayerComponent *component, TimelineStepCacheSide side)
{
	return side == TimelineStepCacheSide::Forward
		? component->timelineFrameWindow.backward
		: component->timelineFrameWindow.forward;
}

static bool PictureMatchesPts(const Picture &picture, int64_t pts, Rational tb)
{
	return HasComparablePts(picture) && ComparePicturePts(picture, pts, tb) == 0;
}

static void RemoveTimelineStepPicture(std::deque<Picture> &cache, int64_t pts, Rational tb)
{
	for (auto it = cache.begin(); it != cache.end(); )
	{
		if (PictureMatchesPts(*it, pts, tb))
		{
			it = cache.erase(it);
		}
		else
		{
			++it;
		}
	}
}

static void CacheTimelinePicture(VideoPlayerComponent *component, const Picture &picture)
{
	if (!component || !component->player || !HasComparablePts(picture))
	{
		return;
	}

	Picture *emptySlot = nullptr;
	for (Picture &cached : component->timelineFrameWindow.recent)
	{
		if (!HasComparablePts(cached))
		{
			if (!emptySlot)
			{
				emptySlot = &cached;
			}
			continue;
		}
		if (ComparePicturePts(cached, picture.GetTimestamp(), picture.GetTimebase()) == 0)
		{
			cached = picture;
			return;
		}
	}

	if (emptySlot)
	{
		*emptySlot = picture;
	}
	else
	{
		component->timelineFrameWindow.recent[component->timelineFrameWindow.recentCursor %
		                                      component->timelineFrameWindow.recent.size()] = picture;
		component->timelineFrameWindow.recentCursor++;
	}
}

static void CacheTimelineStepPicture(VideoPlayerComponent *component, TimelineStepCacheSide side, const Picture &picture)
{
	if (!component || !HasComparablePts(picture))
	{
		return;
	}

	CacheTimelinePicture(component, picture);

	std::deque<Picture> &cache = TimelineStepCache(component, side);
	std::deque<Picture> &opposite = OppositeTimelineStepCache(component, side);
	RemoveTimelineStepPicture(opposite, picture.GetTimestamp(), picture.GetTimebase());

	for (Picture &cached : cache)
	{
		if (PictureMatchesPts(cached, picture.GetTimestamp(), picture.GetTimebase()))
		{
			cached = picture;
			return;
		}
	}

	cache.emplace_back(picture);
	while (cache.size() > TIMELINE_STEP_CACHE_LIMIT)
	{
		cache.pop_front();
	}
}

static Picture TakeTimelineStepPicture(VideoPlayerComponent *component, TimelineStepCacheSide side, int64_t pts, Rational tb)
{
	if (!component)
	{
		return {};
	}

	std::deque<Picture> &cache = TimelineStepCache(component, side);
	for (auto it = cache.begin(); it != cache.end(); ++it)
	{
		if (PictureMatchesPts(*it, pts, tb))
		{
			Picture picture = *it;
			cache.erase(it);
			return picture;
		}
	}
	return {};
}

static Picture FindTimelinePictureCache(VideoPlayerComponent *component, int64_t pts, Rational tb)
{
	if (!component || !component->player)
	{
		return {};
	}

	if (HasComparablePts(component->currentPicture) &&
	    ComparePicturePts(component->currentPicture, pts, tb) == 0)
	{
		return component->currentPicture;
	}

	for (Picture &cached : component->timelineFrameWindow.recent)
	{
		if (PictureMatchesPts(cached, pts, tb))
		{
			return cached;
		}
	}
	return {};
}

static void ClearTimelineStepCaches(VideoPlayerComponent *component)
{
	if (!component)
	{
		return;
	}
	component->timelineFrameWindow.ClearStep();
}

static void MoveCurrentToTimelineStepCache(VideoPlayerComponent *component, bool toForward)
{
	if (!component || !HasComparablePts(component->currentPicture))
	{
		return;
	}

	if (toForward)
	{
		CacheTimelineStepPicture(component, TimelineStepCacheSide::Forward, component->currentPicture);
	}
	else
	{
		CacheTimelineStepPicture(component, TimelineStepCacheSide::Backward, component->currentPicture);
	}
}

static bool TimelineQueueAlreadyPastTarget(VideoPlayerComponent *component, int64_t pts, Rational tb)
{
	if (!component || !component->player)
	{
		return false;
	}

	Picture picture = component->GetPicture();
	if (!picture)
	{
		return false;
	}

	return ComparePicturePts(picture, pts, tb) > 0;
}

static int64_t EstimateTimelineFrameDistance(const Picture &picture, int64_t pts, Rational tb, Animator *animator)
{
	if (!HasComparablePts(picture) || pts == std::numeric_limits<int64_t>::min() || tb.denominator == 0)
	{
		return std::numeric_limits<int64_t>::max();
	}

	double fps = 30.0;
	if (animator && animator->Framerate.numerator > 0 && animator->Framerate.denominator > 0)
	{
		fps = (double)animator->Framerate.numerator / (double)animator->Framerate.denominator;
	}

	long double src = PtsSeconds(picture.GetTimestamp(), picture.GetTimebase());
	long double dst = PtsSeconds(pts, tb);
	return (int64_t)std::llround(std::fabs((double)(src - dst)) * fps);
}

static void PrimeTimelineForwardCache(VideoPlayerComponent *component, size_t budget)
{
	if (!component || !component->player || budget == 0)
	{
		return;
	}

	for (size_t i = 0; i < budget; ++i)
	{
		Picture next = component->GetPicture();
		if (!next)
		{
			break;
		}
		CacheTimelinePicture(component, next);
		CacheTimelineStepPicture(component, TimelineStepCacheSide::Forward, next);
		component->PopPicture();
	}
}

} // anonymous namespace


// ---------------------------------------------------------------------------
// VideoPlayerContext
// ---------------------------------------------------------------------------
struct VideoPlayerContext : public IClass
{
public:
	VideoPlayerContext(int cacheSize, VideoPlayerMode mode);

    ~VideoPlayerContext();

	CodecError Open(const String &path, int cacheSize = 3, const Vision::DecodingPreference &preference = Vision::DecodingPreference::Auto, VideoPlayerMode mode = VideoPlayerMode::Playing, StreamEnabledFlags flags = StreamEnabledFlags::None, bool startImmediately = true);

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

	// Playback-mode ffplay threads
	void ReadThreadPlayback();
	void VideoDecodeThreadPlayback();
	void AudioDecodeThreadPlayback();
	void HandleSeekInReadThread();

	double GetMasterClock();

	void UpdateAudioClockFromOutput(double callbackTime);

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
		return vs.eof.load();
    }

    void SetPause(bool enabled)
    {
		if (!enabled && vs.pause.load())
		{
			double now = SteadyTimeSeconds();
			vs.vidclk.ptsDrift += now - vs.vidclk.lastUpdated;
			SetClock(vs.vidclk, GetClock(vs.vidclk), vs.vidclk.serial);
		}
		vs.pause.store(enabled);
		vs.vidclk.paused = enabled;
		vs.audclk.paused = enabled;
		vs.extclk.paused = enabled;

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

    ConcurrentQueue<Picture> audioFramesLegacy;

    ConcurrentQueue<Picture> subtitles;

    Picture picture;

    Picture audioFrame;

    Picture outputAudioFrame;

    /// Protects `outputAudioFrame`, `unconsumedSamples` and the associated
    /// output audio clock fields. AudioMixer pulls samples from the audio
    /// callback thread while the demux/read thread may seek and reset the
    /// cached frame, so these fields must be updated atomically as a unit.
    std::mutex outputAudioMutex;

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

    uint32_t unconsumedSamples = 0;

    int pictureSize = 0;

    VideoDecodeCallbacks callbacks{};

    double externalClock = -1.0f;

    int64_t lastAudioTimestamp = 0;

    bool audioDeviceChanged = false;

    /// When true, the system AudioStream is not created.  Owners should pull
    /// decoded audio through `GetAudioData(...)` (e.g. Montage's AudioMixer).
    bool audioMixerMode = false;

    std::atomic_bool muted{false};
    std::atomic<float> volume{1.0f};

    /// Timeline compositor enables this so exact seek / step uses the decoded-frame caches;
    /// normal playback still follows the ffplay-style display clock.
    bool timelineDecodeDrive = false;

    /// Becomes true once `SetAudioOutputSpec(...)` has been called with a
    /// successful `SampleConverter::SetOptions`. The audio decode thread for
    /// `audioMixerMode` waits on this before producing frames so that it
    /// doesn't race with the (re)allocation of the SwrContext or push frames
    /// in the wrong output format.
    std::atomic_bool audioSpecReady{false};

    std::mutex audioSpecMutex;

    std::condition_variable audioSpecCv;

    /// Serializes access to `sampleConverter` between the audio decode thread
    /// and the owner thread that calls `SetAudioOutputSpec(...)`.
    std::mutex sampleConverterMutex;

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

	// --- ffplay-style state ---
	FfplayVideoState vs;

	Thread videoDecodeThread;
	Thread audioDecodeThread;

	ConcurrentQueue<AudioFrameSlot> audioFrameSlots;

	std::atomic<int> playbackVideoDecodeBusy{ 0 };
	std::atomic<int> playbackAudioDecodeBusy{ 0 };

	std::mutex          playbackPictureCapMutex;
	std::condition_variable playbackPictureCapCv;

	std::mutex          playbackAudioCapMutex;
	std::condition_variable playbackAudioCapCv;

	int lastDequeuedAudioPacketSerial = -1;
	int outputAudioPacketSerial       = -1;

	double audioCallbackTime = 0.0;

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

			Picture sourcePicture = picture;
			SamplingFactor factors[SamplingFactor::kMaxSublayer] = {};
			GetSamplingFactor(format, factors);
			auto &output = filterGraph->QueryOutputs();
			picture = Picture{output[0]->GetWidth() << factors[0].x, output[0]->GetHeight() << factors[0].y, format, true};
			picture.SetTimestamp(sourcePicture.GetTimestamp());
			picture.SetTimebase(sourcePicture.GetTimebase());
			picture.SetSampleAspectRatio(sourcePicture.GetSampleAspectRatio());
			picture.SetFlags(sourcePicture.GetFlags());
			picture.SetColorSpace(sourcePicture.GetColorSpace());
			picture.SetColorTransferCharacteristic(sourcePicture.GetColorTransferCharacteristic());
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

	if (eof && filterGraph && asyncComputeTaskCount == 0)
	{
		done = true;
		done.notify_one();
	}
}

void VideoPlayerContext::EndOfFile(Vision::Interface::Codec *decoder, const std::function<void(Picture &&)> &callback, MediaType type)
{
	CodedFrame codedFrame{(uint8_t *)nullptr};
	if (filterGraph && type == MediaType::Video)
	{
		done = false;
	}

	auto drainAudioPictures = [&] {
		while (decoder->GetPicture(picture) == CodecError::Success)
		{
			Picture resampled = ResampleAudioFrame(picture);
			if (resampled && resampled.GetWidth() > 0 && resampled.GetFormat() != Format::None)
			{
				callback(std::move(resampled));
				audioSize++;
			}
		}
	};

	CodecError err = CodecError::Again;
	while (err == CodecError::Again)
	{
		// libavcodec may reject the EOF/drain packet with EAGAIN when it still has
		// buffered frames. Drain those frames, then send the null packet again so
		// the decoder actually enters drain mode.
		err = decoder->Decode(codedFrame);
		if (type == MediaType::Video)
		{
			GetPictures(err != CodecError::Again);
		}
		else
		{
			drainAudioPictures();
		}
	}

	if (type == MediaType::Audio && sampleConverter)
	{
		Picture picture = sampleConverter.GetRemainingSamples();
		if (picture && picture.GetWidth() > 0 && picture.GetFormat() != Format::None)
		{
			callback(std::move(picture));
			audioSize++;
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
	// Guard against duplicate starts. `Playback()` / `Transcode()` already wire
	// `task` and then call this method internally; outside callers (e.g. the
	// `VideoPlayerPool` after construction) may call `StartPlay()` a second
	// time. If the demuxer thread is already running or `task` was moved-from
	// (empty), starting another thread would invoke an empty `std::function`
	// from the worker thread and throw `std::bad_function_call`.
	if (!task)
	{
		return;
	}

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
	if (!decoder)
	{
		// No audio stream — still mark the spec as "ready" so the decode
		// thread (if it is somehow waiting) can exit cleanly.
		{
			std::lock_guard lock{ audioSpecMutex };
			audioSpecReady.store(true);
		}
		audioSpecCv.notify_all();
		return;
	}
	Vision::AudioFormatSpec inputSpec = decoder->GetAudioFormat();
	{
		std::lock_guard lock{ sampleConverterMutex };
		if (inputSpec.layout != outputSpec.layout || inputSpec.format != outputSpec.format
		    || inputSpec.sampleRate != outputSpec.sampleRate
		    || inputSpec.numChannel != outputSpec.numChannel)
		{
			if (!sampleConverter.SetOptions(outputSpec, inputSpec))
			{
				CLOG_ERROR("Error when creating sampleConverter. Audio playing maybe corrupted!");
			}
		}
	}

	// In audioMixerMode the AudioDecodeThreadPlayback is parked until the
	// owner publishes a target spec; release it now and discard any frames
	// that may have slipped through before the converter was configured.
	{
		std::lock_guard lock{ audioSpecMutex };
		audioSpecReady.store(true);
	}
	audioSpecCv.notify_all();
}

Picture VideoPlayerContext::ResampleAudioFrame(Picture &picture)
{
	if (!picture || picture.GetWidth() == 0 || picture.GetFormat() == Format::None)
	{
		return {};
	}

	std::lock_guard lock{ sampleConverterMutex };
	if (sampleConverter)
	{
		auto &outputAudioFormat = sampleConverter.GetOutputFormat();
		int rescaledSampleWidth = sampleConverter.RescaleRound(picture.GetWidth());
		if (rescaledSampleWidth <= 0)
		{
			return {};
		}
		Picture rescaledPicture = {(uint32_t) rescaledSampleWidth * outputAudioFormat.numChannel, 1, outputAudioFormat.format, true};
		rescaledPicture.SetWidth(rescaledSampleWidth);
		if (sampleConverter.Convert(rescaledPicture, picture) && rescaledPicture.GetWidth() > 0)
		{
			rescaledPicture.SetSampleRate(outputAudioFormat.sampleRate);
			rescaledPicture.SetTimestamp(picture.GetTimestamp());
			rescaledPicture.SetTimebase(picture.GetTimebase());
			return rescaledPicture;
		}
		return {};
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
	vs.audioHwBufSize = (int)audioStream->FfplayAudioHwBufferBytes();
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
				ClearPictures(audioFramesLegacy, audioFrame, audioSize);
				auto audioDevice = AudioDevice::GetInstance();
				audioStream->Stop();
				audioStream->Reset();
				audioDevice->DestroyAudioStream(&audioStream);
				CreateAudioStream();
				audioDeviceChanged = false;
			}
			audioFramesLegacy.enqueue(ResampleAudioFrame(picture));
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

CodecError VideoPlayerContext::Open(const String &path, int cacheSize, const Vision::DecodingPreference &preference, VideoPlayerMode mode, StreamEnabledFlags flags, bool startImmediately)
{
	if ((flags & StreamEnabledFlags::AudioMixerMode) != 0)
	{
		audioMixerMode = true;
	}
	auto ret = Open(path, preference, flags);
	if (ret != CodecError::Success)
	{
		CLOG_ERROR("Error when opening {}", path);
		return ret;
	}

	if (!startImmediately)
	{
		return CodecError::Success;
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

	static constexpr MediaType kDecoderSlotMediaType[] = {MediaType::Video, MediaType::Audio, MediaType::Subtitle};

	Ref<VideoCodec> *codecs[] = {&decoder, &audioDecoder, &subtitleDecoder};
	std::unique_ptr<ThreadPool> *threadPools[] = {&videoThreadPool, &audioThreadPool, &subtitleThreadPool};
	for (int i = 0; i < SL_ARRAY_LENGTH(codecs); i++)
	{
		if (flags != StreamEnabledFlags::None && !(flags & StreamEnabledFlags(BIT(i))))
		{
			continue;
		}

		CodecInfo info{};
		if (demuxer->GetStreamInfo(kDecoderSlotMediaType[i], info) == CodecError::Success)
		{
			auto decoder = new Vision::FFCodec{};
			decoder->SetPreference(preference);
			if (decoder->OpenDecoder(info) != CodecError::Success)
			{
				return CodecError::FailedToCallDecoder;
			}

			auto animator = decoder->GetAddress<Animator>();
			*animator = demuxer->GetAnimator(kDecoderSlotMediaType[i]);

			*codecs[i] = decoder;
			threadPools[i]->reset(new ThreadPool{1});
		}
	}

	if (auto *ff = demuxer.InterpretAs<Vision::FFFormat>())
	{
		vs.maxFrameDuration = ff->GetMaxFrameDurationForSync();
	}

	return CodecError::Success;
}

// ---------------------------------------------------------------------------
// Playback() — ffplay-style setup
// ---------------------------------------------------------------------------
void VideoPlayerContext::Playback()
{
	if (audioDecoder && !audioMixerMode)
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
		// CreateAudioStream() above already invoked SetAudioOutputSpec, so the
		// decode thread can run unblocked.
		audioSpecReady.store(true);
	}
	else if (!audioDecoder)
	{
		// Timeline / video-only paths open with `StreamEnabledFlags::Video` only. Keeping the
		// default `AudioMaster` sync while `audclk` is never fed from decoded audio makes
		// `GetLivePicture()` stall on `delay` vs a meaningless master clock — top layers can look
		// frozen while another track "carries" timing. Video-master uses frame duration only.
		vs.avSyncType = FfplayAvSyncType::VideoMaster;
		// No audio at all: nothing to wait on.
		audioSpecReady.store(true);
	}
	// In `audioMixerMode`, audioSpecReady stays false until the mixer owner
	// calls `ConfigureMixerSpec(...)` -> `SetAudioOutputSpec(...)`.

	InitClock(vs.audclk, &vs.audioq.serial);
	InitClock(vs.vidclk, &vs.videoq.serial);
	InitClock(vs.extclk, &vs.audioq.serial);

	vs.audioq.Start();
	vs.videoq.Start();

	vs.audioDiffAvgCoef  = exp(log(0.01) / AUDIO_DIFF_AVG_NB);
	vs.audioDiffAvgCount = 0;

	if (decoder)
	{
		videoDecodeThread.Start([this]() { VideoDecodeThreadPlayback(); });
	}
	if (audioDecoder)
	{
		audioDecodeThread.Start([this]() { AudioDecodeThreadPlayback(); });
	}

	task = [this]() { ReadThreadPlayback(); };
	StartPlay();
}

// ---------------------------------------------------------------------------
// ReadThreadPlayback — ffplay read_thread
// ---------------------------------------------------------------------------
void VideoPlayerContext::ReadThreadPlayback()
{
	const int maxVideoPackets = std::max(MIN_FRAMES * 4, kCacheSize * 8);
	const int maxAudioPackets = std::max(MIN_FRAMES * 4, kCacheSize * 8);

	while (true)
	{
		if (state.exited)
		{
			double time = timer.Duration();
			CLOG_INFO("Decoding Statistic: frames:{}, time:{}, fps:{}", frames, time, frames / time);
			break;
		}

		// IMPORTANT: process seek requests BEFORE the pause-sleep, otherwise
		// scrubbing while paused never reaches the demuxer — playback then
		// resumes from the position where pause was applied instead of the
		// scrubbed playhead. (This is the root cause of "play doesn't start
		// from the slider position".)
		if (vs.seekReq.load())
		{
			HandleSeekInReadThread();
			continue;
		}

		if (vs.pause.load())
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			continue;
		}

		bool videoFull = decoder && (vs.videoq.nbPackets >= maxVideoPackets || vs.videoq.sizeBytes >= MAX_QUEUE_SIZE);
		bool audioFull = audioDecoder && (vs.audioq.nbPackets >= maxAudioPackets || vs.audioq.sizeBytes >= MAX_QUEUE_SIZE);
		if (videoFull || audioFull)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			continue;
		}

		Vision::CodedFrame codedFrame;
		auto ret = demuxer->Read(&codedFrame);
		if (ret != CodecError::Success)
		{
			if (ret == CodecError::EndOfFile)
			{
				if (!vs.eof.load())
				{
					if (decoder)
					{
						Vision::CodedFrame nullPkt{ (uint8_t *)nullptr };
						vs.videoq.Put(std::move(nullPkt), 1, SIZE_MAX);
					}
					if (audioDecoder)
					{
						Vision::CodedFrame nullPkt{ (uint8_t *)nullptr };
						vs.audioq.Put(std::move(nullPkt), 1, SIZE_MAX);
					}
				}
				vs.eof.store(true);
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
			}
			else if (ret == CodecError::Again)
			{
				// Unrecognized stream packet — skip
			}
			continue;
		}

		switch (codedFrame.GetType())
		{
		case MediaType::Video:
			if (decoder)
				vs.videoq.Put(std::move(codedFrame), maxVideoPackets, MAX_QUEUE_SIZE);
			break;
		case MediaType::Audio:
			if (audioDecoder)
				vs.audioq.Put(std::move(codedFrame), maxAudioPackets, MAX_QUEUE_SIZE);
			break;
		default:
			break;
		}
	}
}

// ---------------------------------------------------------------------------
// VideoDecodeThreadPlayback — ffplay video_thread + decoder_decode_frame
// ---------------------------------------------------------------------------
void VideoPlayerContext::VideoDecodeThreadPlayback()
{
	while (true)
	{
		Vision::CodedFrame pkt;
		int tag = 0;
		if (vs.videoq.Get(pkt, &tag, true) < 0)
			break;

		playbackVideoDecodeBusy.fetch_add(1);

		int currentSerial = vs.videoq.SerialSnapshot();
		if (tag != currentSerial)
		{
			decoder->Flush();
			playbackVideoDecodeBusy.fetch_sub(1);
			continue;
		}

		auto drainPictures = [&]() {
			Picture pic{};
			while (decoder->GetPicture(pic) == CodecError::Success)
			{
#if IMMORTAL_HAVE_VIDEO_PLAYER_STATISTIC
				frames++;
#endif
				if (colorSpace != ColorSpace::Unspecified)
					pic.SetColorSpace(colorSpace);
				if (colorTransferCharacteristic != ColorTransferCharacteristic::Unspecified)
					pic.SetColorTransferCharacteristic(colorTransferCharacteristic);

				{
					std::unique_lock<std::mutex> lk(playbackPictureCapMutex);
					playbackPictureCapCv.wait(lk, [&] {
						return state.exited || vs.videoq.abortRequest.load()
						    || tag != vs.videoq.SerialSnapshot()
						    || (int)pictures.size() < std::max(MIN_FRAMES, kCacheSize * 2);
					});
				}
				if (state.exited || vs.videoq.abortRequest.load() || tag != vs.videoq.SerialSnapshot())
					break;

				pictures.enqueue(std::move(pic));
				pictureSize++;
			}
		};

		auto shouldStopDraining = [&]() {
			return state.exited || vs.videoq.abortRequest.load() || tag != vs.videoq.SerialSnapshot();
		};

		if (!pkt)
		{
			CodecError err = CodecError::Again;
			while (err == CodecError::Again && !shouldStopDraining())
			{
				err = decoder->Decode(pkt);
				drainPictures();
			}
			playbackVideoDecodeBusy.fetch_sub(1);
			continue;
		}

		CodecError err = decoder->Decode(pkt);
		while (err == CodecError::Again && !shouldStopDraining())
		{
			drainPictures();
			err = decoder->Decode(pkt);
		}
		drainPictures();

		playbackVideoDecodeBusy.fetch_sub(1);
	}
}

// ---------------------------------------------------------------------------
// AudioDecodeThreadPlayback — ffplay audio_thread
// ---------------------------------------------------------------------------
void VideoPlayerContext::AudioDecodeThreadPlayback()
{
	// In audioMixerMode the SwrContext (sampleConverter) is not set up until
	// the owner calls SetAudioOutputSpec(). Block here so we don't decode
	// frames into a stale/uninitialized converter (which would race with
	// SetAudioOutputSpec's swr_alloc/init and produce
	// "Context has not been initialized" errors).
	if (audioMixerMode && !audioSpecReady.load())
	{
		std::unique_lock lk{ audioSpecMutex };
		audioSpecCv.wait(lk, [&] {
			return state.exited || audioSpecReady.load() || vs.audioq.abortRequest.load();
		});
		if (state.exited)
		{
			return;
		}
	}

	while (true)
	{
		Vision::CodedFrame pkt;
		int tag = 0;
		if (vs.audioq.Get(pkt, &tag, true) < 0)
			break;

		playbackAudioDecodeBusy.fetch_add(1);

		int currentSerial = vs.audioq.SerialSnapshot();
		if (tag != currentSerial)
		{
			audioDecoder->Flush();
			playbackAudioDecodeBusy.fetch_sub(1);
			continue;
		}

		auto drainAudio = [&]() {
			Picture pic{};
			while (audioDecoder->GetPicture(pic) == CodecError::Success)
			{
				Picture resampled = ResampleAudioFrame(pic);
				if (!resampled || resampled.GetWidth() == 0 || resampled.GetFormat() == Format::None)
				{
					continue;
				}
				AudioFrameSlot slot;
				slot.picture = std::move(resampled);
				slot.packetSerial = tag;

				{
					std::unique_lock<std::mutex> lk(playbackAudioCapMutex);
					playbackAudioCapCv.wait(lk, [&] {
						return state.exited || vs.audioq.abortRequest.load()
						    || tag != vs.audioq.SerialSnapshot()
						    || (int)audioFrameSlots.size() < MIN_FRAMES * 4;
					});
				}
				if (state.exited || vs.audioq.abortRequest.load() || tag != vs.audioq.SerialSnapshot())
					break;

				audioFrameSlots.enqueue(std::move(slot));
#if IMMORTAL_HAVE_VIDEO_PLAYER_STATISTIC
				audioSize++;
#endif
			}
		};

		auto shouldStopDraining = [&]() {
			return state.exited || vs.audioq.abortRequest.load() || tag != vs.audioq.SerialSnapshot();
		};

		if (!pkt)
		{
			CodecError err = CodecError::Again;
			while (err == CodecError::Again && !shouldStopDraining())
			{
				err = audioDecoder->Decode(pkt);
				drainAudio();
			}
			playbackAudioDecodeBusy.fetch_sub(1);
			continue;
		}

		CodecError err = audioDecoder->Decode(pkt);
		while (err == CodecError::Again && !shouldStopDraining())
		{
			drainAudio();
			err = audioDecoder->Decode(pkt);
		}
		drainAudio();

		playbackAudioDecodeBusy.fetch_sub(1);
	}
}

// ---------------------------------------------------------------------------
// HandleSeekInReadThread
// ---------------------------------------------------------------------------
void VideoPlayerContext::HandleSeekInReadThread()
{
	playbackPictureCapCv.notify_all();
	playbackAudioCapCv.notify_all();

	vs.videoq.Flush();
	vs.audioq.Flush();

	while (playbackVideoDecodeBusy.load() > 0 || playbackAudioDecodeBusy.load() > 0)
	{
		playbackPictureCapCv.notify_all();
		playbackAudioCapCv.notify_all();
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	if (audioStream)
	{
		audioStream->Stop();
		audioStream->Reset();
	}

	if (decoder)
	{
		decoder->Flush();
		ConcurrentQueue<Picture> empty;
		pictures.swap(empty);
		picture = {};
		pictureSize = 0;
	}
	if (audioDecoder)
	{
		audioDecoder->Flush();
		audioFrameSlots.clear();
		audioFrame = {};
		{
			std::lock_guard lock{ outputAudioMutex };
			outputAudioFrame = {};
			unconsumedSamples = 0;
		}
		audioSize = 0;
	}

	vs.audioClock       = NAN;
	vs.audioClockSerial = -1;
	vs.eof.store(false);
	lastDequeuedAudioPacketSerial = -1;
	outputAudioPacketSerial       = -1;

	InitClock(vs.audclk, &vs.audioq.serial);
	InitClock(vs.vidclk, &vs.videoq.serial);
	InitClock(vs.extclk, &vs.audioq.serial);

	demuxer->Seek(vs.seekStreamType, vs.seekPos, vs.seekMin, vs.seekMax);
	vs.seekReq.store(false);

	if (audioStream)
	{
		audioStream->Start();
	}
}

// ---------------------------------------------------------------------------
// GetMasterClock
// ---------------------------------------------------------------------------
double VideoPlayerContext::GetMasterClock()
{
	switch (vs.avSyncType)
	{
	case FfplayAvSyncType::VideoMaster:
		return GetClock(vs.vidclk);
	case FfplayAvSyncType::ExternalClock:
		return GetClock(vs.extclk);
	case FfplayAvSyncType::AudioMaster:
	default:
		return GetClock(vs.audclk);
	}
}

// ---------------------------------------------------------------------------
// UpdateAudioClockFromOutput — ffplay sdl_audio_callback tail
// ---------------------------------------------------------------------------
void VideoPlayerContext::UpdateAudioClockFromOutput(double callbackTime)
{
	if (!std::isnan(vs.audioClock))
	{
		int numChannel = 2;
		if (sampleConverter)
			numChannel = sampleConverter.GetOutputFormat().numChannel;
		int bytesPerFrame = outputAudioFrame ? (int)outputAudioFrame.GetFormat().GetTexelSize() * numChannel : 4;

		int sampleRate = 48000;
		if (outputAudioFrame && outputAudioFrame.GetSampleRate() > 0)
			sampleRate = outputAudioFrame.GetSampleRate();

		int hwBufBytes       = vs.audioHwBufSize;
		int unconsumedBytes  = (int)unconsumedSamples * bytesPerFrame;
		int bytesPerSec      = bytesPerFrame * sampleRate;

		double latencySeconds = 0.0;
		if (bytesPerSec > 0)
			latencySeconds = (double)(2 * hwBufBytes + unconsumedBytes) / (double)bytesPerSec;

		SetClockAt(vs.audclk, vs.audioClock - latencySeconds, vs.audioClockSerial, callbackTime);
		SyncClockToSlave(vs.extclk, vs.audclk);
	}
}

// ---------------------------------------------------------------------------
// Transcode() — unchanged from original
// ---------------------------------------------------------------------------
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
			return state.exited || vs.eof.load() || hasTask;
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
                if (!vs.eof.load())
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
				vs.eof.store(true);
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
						Picture resampled = ResampleAudioFrame(picture);
						if (!resampled || resampled.GetWidth() == 0 || resampled.GetFormat() == Format::None)
						{
							continue;
						}
						callbacks.AudioDecodeFinishSlot(std::move(resampled));
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

// ---------------------------------------------------------------------------
// Seek — for Playback mode, set seekReq; for Transcode mode, use legacy
// ---------------------------------------------------------------------------
void VideoPlayerContext::Seek(MediaType type, int64_t pts, int64_t min, int64_t max)
{
	if (mode == VideoPlayerMode::Playing)
	{
		vs.seekStreamType = type;
		vs.seekPos        = pts;
		vs.seekMin        = min;
		vs.seekMax        = max;
		vs.seekReq.store(true);
		return;
	}

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
		if (vs.eof.load())
		{
			decoder->Flush();
		}
		ClearPictures(pictures, picture, pictureSize);
	}

	if (audioThreadPool)
	{
		audioThreadPool->Join();
		if (vs.eof.load())
		{
			audioDecoder->Flush();
		}
		ClearPictures(audioFramesLegacy, audioFrame, audioSize);
	}

	vs.eof.store(false);
	demuxer->Seek(type, pts, min, max);
	condition.notify_all();

	if (audioStream)
	{
		audioStream->Start();
	}
}

// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
VideoPlayerContext::~VideoPlayerContext()
{
	if (audioStream)
	{
		AudioDevice *audioDevice = AudioDevice::GetInstance();
		if (audioDevice)
		{
			audioDevice->DestroyAudioStream(&audioStream);
		}
		else
		{
			audioStream = nullptr;
		}
	}

    state.exited = true;

	vs.videoq.Abort();
	vs.audioq.Abort();
	playbackPictureCapCv.notify_all();
	playbackAudioCapCv.notify_all();
	audioSpecCv.notify_all();

    condition.notify_all();

    demuxerThread = {};
	videoDecodeThread = {};
	audioDecodeThread = {};

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
		playbackPictureCapCv.notify_all();
    }

	return picture;
}

Picture VideoPlayerContext::GetAudioFrame()
{
    if (audioFrame)
    {
		return audioFrame;
    }

    audioFramesLegacy.try_dequeue(audioFrame);
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
	playbackPictureCapCv.notify_all();
}

void VideoPlayerContext::PopAudioFrame()
{
	lastAudioTimestamp = audioFrame.GetTimestamp();
	audioFrame = {};
	audioSize--;
}

// ---------------------------------------------------------------------------
// GetAudioData — ffplay sdl_audio_callback + audio_decode_frame
// ---------------------------------------------------------------------------
uint32_t VideoPlayerContext::GetAudioData(uint8_t *data, uint32_t samples)
{
	if (vs.pause.load())
		return 0;

	std::lock_guard lock{ outputAudioMutex };
	if (mode != VideoPlayerMode::Playing)
	{
		uint32_t numSamples = WriteAudioData(data, samples);
		while (numSamples != samples)
		{
			outputAudioFrame = GetAudioFrame();
			if (!outputAudioFrame || outputAudioFrame.GetWidth() == 0 || outputAudioFrame.GetFormat() == Format::None)
			{
				outputAudioFrame = {};
				unconsumedSamples = 0;
				break;
			}
			PopAudioFrame();
			unconsumedSamples = outputAudioFrame.GetWidth();
			int numChannel = 2;
			if (sampleConverter)
				numChannel = sampleConverter.GetOutputFormat().numChannel;
			size_t bytePerPixel = outputAudioFrame.GetFormat().GetTexelSize() * numChannel;
			numSamples += WriteAudioData(&data[numSamples * bytePerPixel], samples - numSamples);
		}
		return numSamples;
	}

	audioCallbackTime = SteadyTimeSeconds();

	uint32_t numSamples = WriteAudioData(data, samples);
	while (numSamples != samples)
	{
		AudioFrameSlot slot;
		if (!audioFrameSlots.try_dequeue(slot))
			break;
		playbackAudioCapCv.notify_all();

		int currentSerial = vs.audioq.SerialSnapshot();
		if (slot.packetSerial != currentSerial)
			continue;

		outputAudioPacketSerial = slot.packetSerial;

		outputAudioFrame = std::move(slot.picture);
		if (!outputAudioFrame || outputAudioFrame.GetWidth() == 0 || outputAudioFrame.GetFormat() == Format::None)
		{
			outputAudioFrame = {};
			unconsumedSamples = 0;
			continue;
		}

		unconsumedSamples = outputAudioFrame.GetWidth();
		lastAudioTimestamp = outputAudioFrame.GetTimestamp();

		Rational tb = outputAudioFrame.GetTimebase();
		int sampleRate = outputAudioFrame.GetSampleRate();
		if (sampleRate <= 0)
			sampleRate = 48000;

		if (outputAudioFrame.GetTimestamp() != INT64_MIN && tb.denominator != 0)
		{
			double pts = (double)outputAudioFrame.GetTimestamp() * tb.Normalize();
			vs.audioClock = pts + (double)outputAudioFrame.GetWidth() / (double)sampleRate;
			vs.audioClockSerial = slot.packetSerial;
		}
		else
		{
			vs.audioClock = NAN;
		}

		int numChannel = 2;
		if (sampleConverter)
			numChannel = sampleConverter.GetOutputFormat().numChannel;
		size_t bytePerPixel = outputAudioFrame.GetFormat().GetTexelSize() * numChannel;
		numSamples += WriteAudioData(&data[numSamples * bytePerPixel], samples - numSamples);
	}

	UpdateAudioClockFromOutput(audioCallbackTime);

	return numSamples;
}

uint32_t VideoPlayerContext::WriteAudioData(void *data, uint32_t samples)
{
	uint32_t numSamples = 0;
	if (!data || samples == 0)
	{
		return 0;
	}
	if (unconsumedSamples > 0)
	{
		if (!outputAudioFrame || outputAudioFrame.GetWidth() == 0 || outputAudioFrame.GetFormat() == Format::None)
		{
			outputAudioFrame = {};
			unconsumedSamples = 0;
			return 0;
		}

		uint32_t request = std::min(unconsumedSamples, samples);

        int numChannel = 2;
        if (sampleConverter)
        {
			numChannel = sampleConverter.GetOutputFormat().numChannel;
        }

		size_t bytePerPixel = outputAudioFrame.GetFormat().GetTexelSize() * numChannel;
		if (bytePerPixel == 0 || !outputAudioFrame.GetData())
		{
			outputAudioFrame = {};
			unconsumedSamples = 0;
			return 0;
		}

		auto size = request * bytePerPixel;
        auto consumed = (outputAudioFrame.GetWidth() - unconsumedSamples) * bytePerPixel;
        const float currentVolume = std::clamp(volume.load(), 0.0f, 1.0f);
        if (muted.load() || currentVolume <= 0.0f)
        {
            std::memset(data, 0, size);
        }
        else
        {
		    std::memcpy(data, outputAudioFrame.GetData() + consumed, size);
            ApplyAudioVolume(data, request, numChannel, outputAudioFrame.GetFormat(), currentVolume);
        }

		numSamples += request;
		unconsumedSamples -= request;
	}

	return numSamples;
}

// ===========================================================================
// VideoPlayerComponent
// ===========================================================================

VideoPlayerComponent::VideoPlayerComponent() :
    player{}
{

}

VideoPlayerComponent::VideoPlayerComponent(const String &path, int cacheSize, const Vision::DecodingPreference &preference, VideoPlayerMode mode, StreamEnabledFlags flags) :
    player{}
{
	Open(path, cacheSize, preference, mode, flags);
}

VideoPlayerComponent::~VideoPlayerComponent()
{
    player.Reset();
}

CodecError VideoPlayerComponent::Open(const String &path, int cacheSize, const Vision::DecodingPreference &preference, VideoPlayerMode mode, StreamEnabledFlags flags, bool startImmediately)
{
	player = new VideoPlayerContext{cacheSize, mode};
	CodecError result = player->Open(path, cacheSize, preference, mode, flags, startImmediately);
	if (result != CodecError::Success)
	{
		player = {};
	}
	return result;
}

void VideoPlayerComponent::StartPlay()
{
	player->StartPlay();
}

void VideoPlayerComponent::BeginPlayback()
{
	if (!player)
	{
		return;
	}

	if (player->mode == VideoPlayerMode::Playing)
	{
		player->Playback();
	}
	else if (player->mode == VideoPlayerMode::Transcoding)
	{
		player->Transcode();
	}
}

void VideoPlayerComponent::SetTimelineDecodeDrive(bool enable)
{
	if (!player)
	{
		return;
	}
	player->timelineDecodeDrive = enable;
}

static inline int64_t RescaleTimestamp(int64_t timestamp, int64_t a, int64_t b)
{
	return (timestamp * a + b / 2) / b;
}

// ---------------------------------------------------------------------------
// GetLivePicture — ffplay video_refresh
// ---------------------------------------------------------------------------
Picture VideoPlayerComponent::GetLivePicture()
{
	if (player->mode != VideoPlayerMode::Playing)
	{
		Animator *animator = GetAnimator(MediaType::Video);
		Picture picture = GetPicture();
		if (picture)
		{
			float deltaTime = Time::DeltaTime;
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

	auto &vs = player->vs;

	if (player->timelineDecodeDrive &&
	    timelinePinnedPicturePts != std::numeric_limits<int64_t>::min())
	{
		if (vs.pause.load())
		{
			PrimeTimelineForwardCache(this, TIMELINE_STEP_PREFETCH);
			return currentPicture;
		}
		timelinePinnedPicturePts = std::numeric_limits<int64_t>::min();
		timelinePinnedPictureTimebase = {};
		timelineSeekDirection = 0;
	}

	Animator *animator = GetAnimator(MediaType::Video);
	const bool seekRequestActive =
	    videoSeekTargetPts != std::numeric_limits<int64_t>::min() &&
	    videoSeekTargetTimebase.denominator != 0;

	if (vs.pause.load())
	{
		if (!player->timelineDecodeDrive && !seekRequestActive)
		{
			return currentPicture;
		}
	}

	if (!seekRequestActive && player->audioDecoder && vs.avSyncType == FfplayAvSyncType::AudioMaster
	    && std::isnan(vs.audioClock))
	{
		return currentPicture;
	}

	if (vs.pause.load() && !seekRequestActive)
	{
		return currentPicture;
	}

	Picture picture = player->GetPicture();
	if (!picture)
		return currentPicture;

	double fallbackDur = animator ? animator->SecondsPerFrame : 1.0 / 30.0;
	bool targetReached = false;

	int64_t  picTimestamp = picture.GetTimestamp();
	Rational picTimebase  = picture.GetTimebase();
	double   picPts       = (picTimebase.denominator != 0) ? (double)picTimestamp * picTimebase.Normalize() : NAN;
	if (seekRequestActive)
	{
		while (picture && ComparePicturePts(picture, videoSeekTargetPts, videoSeekTargetTimebase) < 0)
		{
			CacheTimelinePicture(this, picture);
			CacheTimelineStepPicture(this, TimelineStepCacheSide::Backward, picture);
			player->PopPicture();
			picture = player->GetPicture();
			if (!picture)
			{
				return currentPicture;
			}
			picTimestamp = picture.GetTimestamp();
			picTimebase  = picture.GetTimebase();
			picPts       = (picTimebase.denominator != 0) ? (double)picTimestamp * picTimebase.Normalize() : NAN;
		}
		if (picture)
		{
			int cmp = ComparePicturePts(picture, videoSeekTargetPts, videoSeekTargetTimebase);
			if (cmp > 0)
			{
				if (Picture cached = FindTimelinePictureCache(this, videoSeekTargetPts, videoSeekTargetTimebase))
				{
					picture = cached;
					picTimestamp = picture.GetTimestamp();
					picTimebase  = picture.GetTimebase();
					picPts       = (picTimebase.denominator != 0) ? (double)picTimestamp * picTimebase.Normalize() : NAN;
					cmp = 0;
				}
			}
			if (cmp == 0)
			{
				targetReached = true;
			}
		}
		videoSeekTargetPts = std::numeric_limits<int64_t>::min();
		videoSeekTargetTimebase = {};
		videoFrameTimerInit = false;
	}

	if (player->timelineDecodeDrive && (seekRequestActive || targetReached))
	{
		int queueSerial = vs.videoq.SerialSnapshot();
		videoLastDisplayedPts = picPts;
		videoLastDisplayedSerial = queueSerial;
		videoFrameTimerInit = true;
		SetClock(vs.vidclk, picPts, queueSerial);
		SyncClockToSlave(vs.extclk, vs.vidclk);
		player->PopPicture();
		CacheTimelinePicture(this, currentPicture);
		if (seekRequestActive || targetReached)
		{
			if (timelineSeekDirection < 0)
			{
				CacheTimelineStepPicture(this, TimelineStepCacheSide::Forward, currentPicture);
			}
			else
			{
				CacheTimelineStepPicture(this, TimelineStepCacheSide::Backward, currentPicture);
			}
		}
		else
		{
			CacheTimelineStepPicture(this, TimelineStepCacheSide::Backward, currentPicture);
		}
		currentPicture = picture;
		if (seekRequestActive || targetReached)
		{
			if (vs.pause.load())
			{
				timelinePinnedPicturePts = picTimestamp;
				timelinePinnedPictureTimebase = picTimebase;
				PrimeTimelineForwardCache(this, TIMELINE_STEP_PREFETCH);
			}
			else
			{
				timelinePinnedPicturePts = std::numeric_limits<int64_t>::min();
				timelinePinnedPictureTimebase = {};
			}
			timelineSeekDirection = 0;
		}
		return picture;
	}

	int queueSerial = vs.videoq.SerialSnapshot();

	if (queueSerial != lastVideoQueueSerialForFrameTimer)
	{
		videoFrameTimerInit = false;
		lastVideoQueueSerialForFrameTimer = queueSerial;
	}

	double now = SteadyTimeSeconds();

	if (!videoFrameTimerInit)
	{
		videoFrameTimer     = now;
		videoFrameTimerInit = true;
		videoLastDisplayedPts    = picPts;
		videoLastDisplayedSerial = queueSerial;

		SetClock(vs.vidclk, picPts, queueSerial);
		SyncClockToSlave(vs.extclk, vs.vidclk);

		player->PopPicture();
		currentPicture = picture;
		return picture;
	}

	double lastDuration = VpDuration(videoLastDisplayedPts, picPts, vs.maxFrameDuration, fallbackDur);

	double masterClock = player->GetMasterClock();
	double diff = GetClock(vs.vidclk) - masterClock;
	double delay = ComputeTargetDelay(lastDuration, diff, vs);

	if (now < videoFrameTimer + delay)
	{
		return currentPicture;
	}

	videoFrameTimer += delay;
	if (delay > 0 && now - videoFrameTimer > AV_SYNC_THRESHOLD_MAX)
		videoFrameTimer = now;

	videoLastDisplayedPts    = picPts;
	videoLastDisplayedSerial = queueSerial;

	SetClock(vs.vidclk, picPts, queueSerial);
	SyncClockToSlave(vs.extclk, vs.vidclk);

	player->PopPicture();

	int framedrop = vs.framedrop;
	bool canDrop = (framedrop > 0 || (framedrop < 0 && vs.avSyncType != FfplayAvSyncType::VideoMaster));
	if (canDrop)
	{
		Picture next = player->GetPicture();
		if (next)
		{
			int64_t  nextTs = next.GetTimestamp();
			Rational nextTb = next.GetTimebase();
			double   nextPts = (nextTb.denominator != 0) ? (double)nextTs * nextTb.Normalize() : NAN;
			double   nextDur = VpDuration(picPts, nextPts, vs.maxFrameDuration, fallbackDur);
			double   nowAfter = SteadyTimeSeconds();
			if (nowAfter > videoFrameTimer + nextDur)
			{
				vs.frameDropsLate++;
				picture = next;
				picPts = nextPts;

				videoLastDisplayedPts    = picPts;
				SetClock(vs.vidclk, picPts, queueSerial);
				SyncClockToSlave(vs.extclk, vs.vidclk);

				player->PopPicture();
			}
		}
	}

	currentPicture = picture;
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

void VideoPlayerComponent::SeekToFrame(MediaType type, int64_t pts)
{
	Animator *animator = GetAnimator(type);
	if (type == MediaType::Video && animator)
	{
		Rational targetTb = animator->TimebaseRational;
		const bool hasCurrent = HasComparablePts(currentPicture);
		const int currentCmp = hasCurrent ? ComparePicturePts(currentPicture, pts, targetTb) : 0;
		const int seekDirection = currentCmp < 0 ? 1 : currentCmp > 0 ? -1 : 0;

		auto pinPicture = [&](const Picture &picture) {
			if (HasComparablePts(currentPicture) && ComparePicturePts(currentPicture, picture.GetTimestamp(), picture.GetTimebase()) != 0)
			{
				CacheTimelinePicture(this, currentPicture);
				MoveCurrentToTimelineStepCache(this, seekDirection < 0);
			}
			currentPicture = picture;
			videoSeekTargetPts = std::numeric_limits<int64_t>::min();
			videoSeekTargetTimebase = {};
			videoFrameTimerInit = false;
			if (player->vs.pause.load())
			{
				timelinePinnedPicturePts = picture.GetTimestamp();
				timelinePinnedPictureTimebase = picture.GetTimebase();
			}
			else
			{
				timelinePinnedPicturePts = std::numeric_limits<int64_t>::min();
				timelinePinnedPictureTimebase = {};
			}
			timelineSeekDirection = 0;
		};

		Picture cached;
		if (currentCmp > 0)
		{
			cached = TakeTimelineStepPicture(this, TimelineStepCacheSide::Backward, pts, targetTb);
		}
		else if (currentCmp < 0)
		{
			cached = TakeTimelineStepPicture(this, TimelineStepCacheSide::Forward, pts, targetTb);
		}
		if (!cached)
		{
			cached = FindTimelinePictureCache(this, pts, targetTb);
		}
		if (cached)
		{
			pinPicture(cached);
			return;
		}

		const int64_t distance = EstimateTimelineFrameDistance(currentPicture, pts, targetTb, animator);
		const bool discontinuousSeek =
		    distance == std::numeric_limits<int64_t>::max() || distance > TIMELINE_STEP_NEAR_FRAMES;

		if (discontinuousSeek)
		{
			ClearTimelineStepCaches(this);
		}

		if (currentCmp != 0)
		{
			CacheTimelinePicture(this, currentPicture);
			if (!discontinuousSeek)
			{
				MoveCurrentToTimelineStepCache(this, seekDirection < 0);
			}
		}

		videoSeekTargetPts = pts;
		videoSeekTargetTimebase = targetTb;
		videoFrameTimerInit = false;
		timelinePinnedPicturePts = std::numeric_limits<int64_t>::min();
		timelinePinnedPictureTimebase = {};
		timelineSeekDirection = seekDirection;
		currentPicture = {};
	}
	player->Seek(type, pts, std::numeric_limits<int64_t>::min(), std::numeric_limits<int64_t>::max());
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

void VideoPlayerComponent::SetMuted(bool enabled)
{
    if (!player)
    {
        return;
    }
    player->muted.store(enabled);
}

bool VideoPlayerComponent::IsMuted() const
{
    return player ? player->muted.load() : false;
}

void VideoPlayerComponent::SetVolume(float value)
{
    if (!player)
    {
        return;
    }
    player->volume.store(std::clamp(value, 0.0f, 1.0f));
}

float VideoPlayerComponent::GetVolume() const
{
    return player ? player->volume.load() : 1.0f;
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

uint32_t VideoPlayerComponent::PullAudio(uint8_t *data, uint32_t samples)
{
	if (!player)
	{
		return 0;
	}
	return player->GetAudioData(data, samples);
}

void VideoPlayerComponent::ConfigureMixerSpec(const Vision::AudioFormatSpec &outputSpec)
{
	if (player)
	{
		player->audioMixerMode = true;
		player->SetAudioOutputSpec(outputSpec);
	}
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
	if (!player)
	{
		return {};
	}
	auto decoder = player->audioDecoder.InterpretAs<Vision::FFCodec>();
	if (!decoder)
	{
		return {};
	}
	return decoder->GetTimebase();
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
