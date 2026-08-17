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

static bool IsValidAudioSpec(const Vision::AudioFormatSpec &spec)
{
	return spec.sampleRate > 0 && spec.numChannel > 0 && spec.format != Format::None;
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

static void ClearTimelineCaches(VideoPlayerComponent *component)
{
	if (!component)
	{
		return;
	}

	for (Picture &cached : component->timelineFrameWindow.recent)
	{
		cached = {};
	}
	component->timelineFrameWindow.recentCursor = 0;
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

	uint64_t Seek(MediaType type, int64_t pts, int64_t min, int64_t max);

    Picture GetPicture();

    Picture GetAudioFrame();

    void PopPicture();

    void PopAudioFrame();

    void StartPlay();

	void SetAudioOutputSpec(const Vision::AudioFormatSpec &outputSpec);

    void CreateAudioStream();

	CodecError ReopenAudioDecoder();

	void ResetAudioPlaybackState();

	CodecError SwitchAudioTrackLocked(Vision::FFFormat *format, int index);

	int GetAudioOutputChannelCount() const;

    uint32_t GetAudioData(uint8_t *data, uint32_t samples);

	uint32_t WriteAudioData(void *data, uint32_t samples);

    void EndOfFile(Vision::Interface::Codec *decoder, const std::function<void(Picture &&)> &callback, MediaType type);

	void GetPictures(bool eof);

	void Join();

	Picture ResampleAudioFrame(Picture &picture);

	bool PrepareAudioFrameAfterSeek(Picture &picture, int packetSerial, uint32_t &firstSampleOffset);

	void SetAudioSeekTarget(MediaType type, int64_t pts, int packetSerial);

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

    bool IsPlaybackDrained(MediaType type) const
    {
        if (!vs.eof.load(std::memory_order_acquire))
        {
            return false;
        }

        if (type == MediaType::Video)
        {
            const bool decoderIdle = mode == VideoPlayerMode::Playing
                ? playbackVideoDecoderDrained.load(std::memory_order_acquire) &&
                    playbackVideoDecodeBusy.load(std::memory_order_acquire) == 0 &&
                    vs.videoq.PacketCount() == 0
                : !videoThreadPool || videoThreadPool->TaskSize().load(std::memory_order_acquire) == 0;
            return decoderIdle && pictures.empty() && !picture;
        }

        if (type == MediaType::Audio)
        {
            const bool decoderIdle = mode == VideoPlayerMode::Playing
                ? playbackAudioDecoderDrained.load(std::memory_order_acquire) &&
                    playbackAudioDecodeBusy.load(std::memory_order_acquire) == 0 &&
                    vs.audioq.PacketCount() == 0
                : !audioThreadPool || audioThreadPool->TaskSize().load(std::memory_order_acquire) == 0;
            if (!decoderIdle || !audioFrameSlots.empty())
            {
                return false;
            }

            std::lock_guard lock{ outputAudioMutex };
            return unconsumedSamples == 0;
        }

        return false;
    }

    bool HasPendingSeek() const
    {
        return vs.seekReq.load(std::memory_order_acquire) ||
            playbackAppliedSeekSerial.load(std::memory_order_acquire) <
                playbackSeekRequestSerial.load(std::memory_order_acquire);
    }

    bool IsSeekApplied(uint64_t serial) const
    {
        return serial == 0 ||
            playbackAppliedSeekSerial.load(std::memory_order_acquire) >= serial;
    }

    void SetPause(bool enabled)
    {
		const bool wasPaused = vs.pause.load(std::memory_order_acquire);
		if (wasPaused == enabled)
		{
			return;
		}

		if (!enabled && wasPaused)
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

    CodecError SwitchTrack(MediaType mediaType, int index);

	CodecError RequestAudioTrackSwitch(int index);

	bool TakePendingAudioTrackSwitch(int &index);

	bool ProcessPendingAudioTrackSwitch();

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

	void SetSubtitlePreviewEnabled(bool enabled);

	Vision::SubtitleCue GetCurrentSubtitleCue(double seconds);

	CodecError ReopenSubtitleDecoder();

	CodecError SwitchSubtitleTrackLocked(Vision::FFFormat *format, int index);

	void ResetSubtitlePlaybackState();

	void SubtitleDecodeThreadPlayback();

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
	mutable std::mutex outputAudioMutex;

    int kCacheSize;

    struct State
    {
        bool playing = false;
		std::atomic_bool exited{ false };
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

	std::atomic<int64_t> lastAudioTimestamp{ INT64_MIN };

    bool audioDeviceChanged = false;

    /// When true, the system AudioStream is not created.  Owners should pull
    /// decoded audio through `GetAudioData(...)` (e.g. Montage's AudioMixer).
	bool audioMixerMode = false;

	Vision::DecodingPreference decodingPreference = Vision::DecodingPreference::Auto;

	Vision::AudioFormatSpec audioOutputSpec{};

	std::atomic<int> audioOutputNumChannel{2};

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
	Thread subtitleDecodeThread;

	ConcurrentQueue<AudioFrameSlot> audioFrameSlots;

	std::atomic<int> playbackVideoDecodeBusy{ 0 };
	std::atomic<int> playbackAudioDecodeBusy{ 0 };
	std::atomic<int> playbackSubtitleDecodeBusy{ 0 };
	std::atomic_bool playbackVideoDecoderDrained{ false };
	std::atomic_bool playbackAudioDecoderDrained{ false };

	std::atomic_bool subtitlePreviewEnabled{ false };
	std::mutex subtitleCueMutex;
	std::deque<Vision::SubtitleCue> subtitleCues;

	std::atomic_bool playbackReadThreadActive{ false };

	std::mutex playbackSeekMutex;
	std::atomic_uint64_t playbackSeekRequestSerial{ 0 };
	std::atomic_uint64_t playbackAppliedSeekSerial{ 0 };
	std::atomic<int64_t> audioSeekTargetUs{ INT64_MIN };
	std::atomic<int> audioSeekTargetSerial{ -1 };

	std::mutex pendingAudioTrackSwitchMutex;
	int pendingAudioTrackSwitchIndex = -1;
	std::atomic_bool pendingAudioTrackSwitch{ false };

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
			if (format.IsType(Format::YUV) && !scaleFilter)
			{
				auto &format = picture.GetFormat();
				Format dstFormat = Format::RGBA8;
				bool isHighBitDepth = format.IsType(Format::_10Bits) || format.IsType(Format::_12Bits);
				dstFormat = isHighBitDepth ? Format::R16G16B16A16_UNORM : Format::RGBA8;
				scaleFilter = filterGraph->Insert<ScaleFilter>(0, picture.GetFormat(), dstFormat, picture.GetWidth(), picture.GetHeight());
			}

			asyncComputeTaskCount.fetch_add(1, std::memory_order_acq_rel);
			AsyncRecordingScope computeBatch{ asyncComputeThread };
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
			asyncComputeThread->Execute<ExecutionCompletedTask>([=, this]() {
				auto completeTask = [this, eof] {
					const int remaining = asyncComputeTaskCount.fetch_sub(1, std::memory_order_acq_rel) - 1;
					if (eof && remaining == 0)
					{
						done = true;
						done.notify_one();
					}
					condition.notify_all();
				};

				try
				{
					callbacks.VideoDecodeFinishSlot(std::move((Picture &&)picture));
				}
				catch (...)
				{
					completeTask();
					throw;
				}
				completeTask();
			});
			if (state.exited)
			{
				break;
			}
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

	Picture remainingAudio;
	if (type == MediaType::Audio)
	{
		std::lock_guard lock{ sampleConverterMutex };
		if (sampleConverter)
		{
			remainingAudio = sampleConverter.GetRemainingSamples();
		}
	}
	if (remainingAudio && remainingAudio.GetWidth() > 0 && remainingAudio.GetFormat() != Format::None)
	{
		callback(std::move(remainingAudio));
		audioSize++;
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

CodecError VideoPlayerContext::ReopenAudioDecoder()
{
	CodecInfo info{};
	CodecError ret = demuxer->GetStreamInfo(MediaType::Audio, info);
	if (ret != CodecError::Success)
	{
		audioDecoder = {};
		return ret;
	}

	auto nextDecoder = new Vision::FFCodec{};
	nextDecoder->SetPreference(decodingPreference);
	ret = nextDecoder->OpenDecoder(info);
	if (ret != CodecError::Success)
	{
		audioDecoder = {};
		return ret;
	}

	auto animator = nextDecoder->GetAddress<Animator>();
	*animator = demuxer->GetAnimator(MediaType::Audio);
	audioDecoder = nextDecoder;

	if (!audioThreadPool && mode != VideoPlayerMode::Playing)
	{
		audioThreadPool.reset(new ThreadPool{1});
	}

	return CodecError::Success;
}

void VideoPlayerContext::ResetAudioPlaybackState()
{
	audioSeekTargetSerial.store(-1, std::memory_order_release);
	audioFrameSlots.clear();
	ClearPictures(audioFramesLegacy, audioFrame, audioSize);
	{
		std::lock_guard lock{ outputAudioMutex };
		outputAudioFrame = {};
		unconsumedSamples = 0;
		vs.audioClock = NAN;
		vs.audioClockSerial = -1;
		lastDequeuedAudioPacketSerial = -1;
		outputAudioPacketSerial = -1;
		lastAudioTimestamp = INT64_MIN;
	}
	{
		std::lock_guard lock{ sampleConverterMutex };
		sampleConverter.Reset();
	}

	vs.eof.store(false);
	playbackAudioDecoderDrained.store(false, std::memory_order_release);
	InitClock(vs.audclk, &vs.audioq.serial);
	SyncClockToSlave(vs.extclk, vs.audclk);
}

CodecError VideoPlayerContext::SwitchAudioTrackLocked(Vision::FFFormat *format, int index)
{
	CodecError ret = format->SwitchTrack(MediaType::Audio, index);
	if (ret != CodecError::Success)
	{
		return ret;
	}

	if (audioMixerMode)
	{
		std::lock_guard lock{ audioSpecMutex };
		audioSpecReady.store(false);
	}

	if (audioStream)
	{
		audioStream->Stop();
		audioStream->Reset();
	}

	if (mode == VideoPlayerMode::Playing)
	{
		playbackAudioCapCv.notify_all();
		vs.audioq.Flush();
		while (playbackAudioDecodeBusy.load() > 0)
		{
			playbackAudioCapCv.notify_all();
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
	}
	else if (audioThreadPool)
	{
		audioThreadPool->RemoveTasks();
		audioThreadPool->Join();
	}

	if (audioDecoder)
	{
		audioDecoder->Flush();
	}
	ResetAudioPlaybackState();
	double videoClock = GetClock(vs.vidclk);
	if (!std::isnan(videoClock))
	{
		const int audioSerial = vs.audioq.SerialSnapshot();
		vs.audioClock = videoClock;
		vs.audioClockSerial = audioSerial;
		SetClock(vs.audclk, videoClock, audioSerial);
		SyncClockToSlave(vs.extclk, vs.audclk);
	}

	ret = ReopenAudioDecoder();
	if (ret != CodecError::Success)
	{
		if (audioStream)
		{
			audioStream->Start();
		}
		return ret;
	}

	if (!audioMixerMode && !audioStream && mode == VideoPlayerMode::Playing)
	{
		AudioDevice *audioDevice = AudioDevice::GetInstance();
		if (audioDevice)
		{
			CreateAudioStream();
		}
	}
	else if (IsValidAudioSpec(audioOutputSpec))
	{
		SetAudioOutputSpec(audioOutputSpec);
	}

	if (!audioMixerMode)
	{
		audioSpecReady.store(true);
	}

	if (audioStream)
	{
		audioStream->Start();
	}
	condition.notify_all();
	playbackAudioCapCv.notify_all();
	audioSpecCv.notify_all();

	return CodecError::Success;
}

CodecError VideoPlayerContext::ReopenSubtitleDecoder()
{
	CodecInfo info{};
	CodecError ret = demuxer->GetStreamInfo(MediaType::Subtitle, info);
	if (ret != CodecError::Success)
	{
		subtitleDecoder = {};
		return ret;
	}

	auto nextDecoder = new Vision::FFCodec{};
	nextDecoder->SetPreference(Vision::DecodingPreference::Software);
	ret = nextDecoder->OpenDecoder(info);
	if (ret != CodecError::Success)
	{
		subtitleDecoder = {};
		return ret;
	}

	subtitleDecoder = nextDecoder;
	return CodecError::Success;
}

void VideoPlayerContext::ResetSubtitlePlaybackState()
{
	{
		std::lock_guard lock{ subtitleCueMutex };
		subtitleCues.clear();
	}
	vs.subtitleq.Flush();
	while (playbackSubtitleDecodeBusy.load() > 0)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	if (subtitleDecoder)
	{
		subtitleDecoder->Flush();
	}
}

CodecError VideoPlayerContext::SwitchSubtitleTrackLocked(Vision::FFFormat *format, int index)
{
	CodecError ret = format->SwitchTrack(MediaType::Subtitle, index);
	if (ret != CodecError::Success)
	{
		return ret;
	}

	ResetSubtitlePlaybackState();
	ret = ReopenSubtitleDecoder();
	condition.notify_all();
	return ret;
}

void VideoPlayerContext::SetSubtitlePreviewEnabled(bool enabled)
{
	const bool previous = subtitlePreviewEnabled.exchange(enabled, std::memory_order_acq_rel);
	if (previous == enabled)
	{
		return;
	}

	ResetSubtitlePlaybackState();
	condition.notify_all();
}

Vision::SubtitleCue VideoPlayerContext::GetCurrentSubtitleCue(double seconds)
{
	if (!subtitlePreviewEnabled.load(std::memory_order_acquire))
	{
		return {};
	}

	static constexpr double SubtitleCueToleranceSeconds = 0.25;
	std::lock_guard lock{ subtitleCueMutex };
	subtitleCues.erase(
		std::remove_if(
			subtitleCues.begin(),
			subtitleCues.end(),
			[seconds](const Vision::SubtitleCue &cue) {
				return cue.endSeconds + SubtitleCueToleranceSeconds < seconds;
			}),
		subtitleCues.end());

	Vision::SubtitleCue current{};
	for (const Vision::SubtitleCue &cue : subtitleCues)
	{
		if (seconds >= cue.startSeconds - SubtitleCueToleranceSeconds &&
		    seconds <= cue.endSeconds + SubtitleCueToleranceSeconds &&
		    (!current || cue.startSeconds >= current.startSeconds))
		{
			current = cue;
		}
	}
	return current;
}

CodecError VideoPlayerContext::RequestAudioTrackSwitch(int index)
{
	if (index < 0)
	{
		return CodecError::InvalidArguments;
	}

	{
		std::lock_guard lock{ pendingAudioTrackSwitchMutex };
		pendingAudioTrackSwitchIndex = index;
	}
	pendingAudioTrackSwitch.store(true, std::memory_order_release);

	vs.audioq.Flush();
	playbackAudioCapCv.notify_all();
	condition.notify_all();
	audioSpecCv.notify_all();

	return CodecError::Success;
}

bool VideoPlayerContext::TakePendingAudioTrackSwitch(int &index)
{
	if (!pendingAudioTrackSwitch.exchange(false, std::memory_order_acq_rel))
	{
		return false;
	}

	std::lock_guard lock{ pendingAudioTrackSwitchMutex };
	index = pendingAudioTrackSwitchIndex;
	pendingAudioTrackSwitchIndex = -1;
	return index >= 0;
}

bool VideoPlayerContext::ProcessPendingAudioTrackSwitch()
{
	int index = -1;
	if (!TakePendingAudioTrackSwitch(index))
	{
		return false;
	}

	auto *format = demuxer.InterpretAs<Vision::FFFormat>();
	if (!format)
	{
		return true;
	}

	std::unique_lock lock{ mutex.demux };
	CodecError ret = SwitchAudioTrackLocked(format, index);
	if (ret != CodecError::Success)
	{
		CLOG_ERROR("Failed to switch audio track {}", index);
	}
	return true;
}

CodecError VideoPlayerContext::SwitchTrack(MediaType mediaType, int index)
{
	auto *format = demuxer.InterpretAs<Vision::FFFormat>();
	if (!format)
	{
		return CodecError::ExternalFailed;
	}

	if (mediaType == MediaType::Audio)
	{
		if (mode == VideoPlayerMode::Playing && playbackReadThreadActive.load(std::memory_order_acquire))
		{
			return RequestAudioTrackSwitch(index);
		}

		std::unique_lock lock{ mutex.demux };
		return SwitchAudioTrackLocked(format, index);
	}
	if (mediaType == MediaType::Subtitle)
	{
		std::unique_lock lock{ mutex.demux };
		return SwitchSubtitleTrackLocked(format, index);
	}

	std::unique_lock lock{ mutex.demux };
	return format->SwitchTrack(mediaType, index);
}

int VideoPlayerContext::GetAudioOutputChannelCount() const
{
	return std::max(1, audioOutputNumChannel.load());
}

void VideoPlayerContext::SetAudioOutputSpec(const Vision::AudioFormatSpec &outputSpec)
{
	audioOutputSpec = outputSpec;
	audioOutputNumChannel.store(outputSpec.numChannel > 0 ? outputSpec.numChannel : 2);

	auto decoder = audioDecoder.InterpretAs<Vision::FFCodec>();
	if (!decoder)
	{
		// No audio stream — still mark the spec as "ready" so the decode
		// thread (if it is somehow waiting) can exit cleanly.
		{
			std::lock_guard lock{ sampleConverterMutex };
			sampleConverter.Reset();
		}
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
		else
		{
			sampleConverter.Reset();
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

void VideoPlayerContext::SetAudioSeekTarget(MediaType type, int64_t pts, int packetSerial)
{
	audioSeekTargetSerial.store(-1, std::memory_order_release);
	if (!audioDecoder || (type != MediaType::Audio && type != MediaType::Video))
	{
		return;
	}

	Animator &animator = demuxer->GetAnimator(type);
	const Rational timebase = animator.TimebaseRational;
	if (timebase.numerator <= 0 || timebase.denominator <= 0)
	{
		return;
	}

	int64_t targetUs = Vision::RationalRescale(pts, timebase, { 1, 1000000 });
	targetUs = std::max<int64_t>(0, targetUs);
	audioSeekTargetUs.store(targetUs, std::memory_order_relaxed);
	audioSeekTargetSerial.store(packetSerial, std::memory_order_release);
}

bool VideoPlayerContext::PrepareAudioFrameAfterSeek(
	Picture &picture,
	int packetSerial,
	uint32_t &firstSampleOffset)
{
	firstSampleOffset = 0;
	if (audioSeekTargetSerial.load(std::memory_order_acquire) != packetSerial)
	{
		return true;
	}

	auto finishSeekPreroll = [&] {
		int expectedSerial = packetSerial;
		audioSeekTargetSerial.compare_exchange_strong(
			expectedSerial,
			-1,
			std::memory_order_acq_rel,
			std::memory_order_acquire);
	};

	const Rational timebase = picture.GetTimebase();
	const int64_t timestamp = picture.GetTimestamp();
	const uint32_t sampleRate = picture.GetSampleRate();
	const uint32_t sampleCount = picture.GetWidth();
	if (timestamp == INT64_MIN || timebase.numerator <= 0 || timebase.denominator <= 0 ||
		sampleRate == 0 || sampleCount == 0)
	{
		finishSeekPreroll();
		return true;
	}

	const int64_t targetUs = audioSeekTargetUs.load(std::memory_order_relaxed);
	const int64_t frameStartUs = Vision::RationalRescale(timestamp, timebase, { 1, 1000000 });
	const long double frameDurationUs =
		(long double)sampleCount * 1000000.0L / (long double)sampleRate;
	const long double frameEndUs = (long double)frameStartUs + frameDurationUs;
	if (frameEndUs <= (long double)targetUs)
	{
		return false;
	}

	if (frameStartUs < targetUs)
	{
		const long double samplesToSkip = std::ceil(
			(long double)(targetUs - frameStartUs) * (long double)sampleRate / 1000000.0L);
		if (samplesToSkip >= (long double)sampleCount)
		{
			return false;
		}

		firstSampleOffset = (uint32_t)std::max<long double>(0.0L, samplesToSkip);
		const int64_t skippedPts = Vision::RationalRescale(
			firstSampleOffset,
			{ 1, (int64_t)sampleRate },
			timebase);
		picture.SetTimestamp(timestamp + skippedPts);
	}

	finishSeekPreroll();
	return true;
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

	const String &source = demuxer->GetSource();
	audioStream->SetDebugName(std::string{ source.c_str(), source.size() });
	vs.audioHwBufSize = (int)audioStream->FfplayAudioHwBufferBytes();
}

void VideoPlayerContext::Join()
{
	if (mode != VideoPlayerMode::Transcoding)
	{
		return;
	}

	condition.notify_all();
	demuxerThread.Join();
	if (videoThreadPool)
	{
		videoThreadPool->Join();
	}
	if (audioThreadPool)
	{
		audioThreadPool->Join();
	}
	if (subtitleThreadPool)
	{
		subtitleThreadPool->Join();
	}
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
	decodingPreference = preference;

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
			decoder->SetPreference(kDecoderSlotMediaType[i] == MediaType::Subtitle ? Vision::DecodingPreference::Software : preference);
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
	playbackVideoDecoderDrained.store(false, std::memory_order_release);
	playbackAudioDecoderDrained.store(false, std::memory_order_release);

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
	vs.subtitleq.Start();

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
	if (subtitleDecoder)
	{
		subtitleDecodeThread.Start([this]() { SubtitleDecodeThreadPlayback(); });
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
	const int maxSubtitlePackets = std::max(8, kCacheSize * 4);

	playbackReadThreadActive.store(true, std::memory_order_release);
	while (true)
	{
		if (state.exited)
		{
			double time = timer.Duration();
			CLOG_INFO("Decoding Statistic: frames:{}, time:{}, fps:{}", frames, time, frames / time);
			break;
		}

		if (ProcessPendingAudioTrackSwitch())
		{
			continue;
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
		bool subtitleFull = subtitlePreviewEnabled.load(std::memory_order_acquire) &&
			subtitleDecoder && vs.subtitleq.nbPackets >= maxSubtitlePackets;
		if (videoFull || audioFull || subtitleFull)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			continue;
		}

		Vision::CodedFrame codedFrame;
		CodecError ret;
		MediaType codedFrameType = MediaType::Data;
		int videoQueueSerial = vs.videoq.SerialSnapshot();
		int audioQueueSerial = vs.audioq.SerialSnapshot();
		int subtitleQueueSerial = vs.subtitleq.SerialSnapshot();
		bool queueVideoPacket = false;
		bool queueAudioPacket = false;
		bool queueSubtitlePacket = false;
		{
			std::unique_lock lock{ mutex.demux };
			videoQueueSerial = vs.videoq.SerialSnapshot();
			audioQueueSerial = vs.audioq.SerialSnapshot();
			subtitleQueueSerial = vs.subtitleq.SerialSnapshot();
			ret = demuxer->Read(&codedFrame);
			if (ret == CodecError::Success)
			{
				codedFrameType = codedFrame.GetType();
				switch (codedFrameType)
				{
				case MediaType::Video:
					queueVideoPacket = decoder.Get() != nullptr;
					break;
				case MediaType::Audio:
					queueAudioPacket = audioDecoder.Get() != nullptr;
					break;
				case MediaType::Subtitle:
					queueSubtitlePacket =
						subtitlePreviewEnabled.load(std::memory_order_acquire) &&
						subtitleDecoder.Get() != nullptr;
					break;
				default:
					break;
				}
			}
			else if (ret == CodecError::EndOfFile)
			{
				if (!vs.eof.load())
				{
					queueVideoPacket = decoder.Get() != nullptr;
					queueAudioPacket = audioDecoder.Get() != nullptr;
					queueSubtitlePacket =
						subtitlePreviewEnabled.load(std::memory_order_acquire) &&
						subtitleDecoder.Get() != nullptr;
				}
				vs.eof.store(true);
			}
		}
		if (ret == CodecError::Success)
		{
			switch (codedFrameType)
			{
			case MediaType::Video:
				if (queueVideoPacket)
					vs.videoq.PutIfSerial(std::move(codedFrame), maxVideoPackets, MAX_QUEUE_SIZE, videoQueueSerial);
				break;
			case MediaType::Audio:
				if (queueAudioPacket)
					vs.audioq.PutIfSerial(std::move(codedFrame), maxAudioPackets, MAX_QUEUE_SIZE, audioQueueSerial);
				break;
			case MediaType::Subtitle:
				if (queueSubtitlePacket)
					vs.subtitleq.PutIfSerial(std::move(codedFrame), maxSubtitlePackets, MAX_QUEUE_SIZE, subtitleQueueSerial);
				break;
			default:
				break;
			}
			continue;
		}
		if (ret == CodecError::EndOfFile)
		{
			if (queueVideoPacket)
			{
				Vision::CodedFrame nullPkt{ (uint8_t *)nullptr };
				vs.videoq.PutIfSerial(std::move(nullPkt), maxVideoPackets, SIZE_MAX, videoQueueSerial);
			}
			if (queueAudioPacket)
			{
				Vision::CodedFrame nullPkt{ (uint8_t *)nullptr };
				vs.audioq.PutIfSerial(std::move(nullPkt), maxAudioPackets, SIZE_MAX, audioQueueSerial);
			}
			if (queueSubtitlePacket)
			{
				Vision::CodedFrame nullPkt{ (uint8_t *)nullptr };
				vs.subtitleq.PutIfSerial(std::move(nullPkt), maxSubtitlePackets, SIZE_MAX, subtitleQueueSerial);
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			continue;
		}
		if (ret == CodecError::Again)
		{
			// The demuxer can temporarily return EAGAIN/EINTR. Yield instead of
			// converting a transient read condition into end-of-stream.
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			continue;
		}

		// A damaged packet or temporary I/O failure is not EOF. Back off so a
		// persistent source error cannot turn the read thread into a busy loop.
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	}
	playbackReadThreadActive.store(false, std::memory_order_release);
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
			playbackVideoDecoderDrained.store(true, std::memory_order_release);
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
	auto waitForAudioSpec = [&]() -> bool {
		if (!audioMixerMode || audioSpecReady.load())
		{
			return true;
		}

		std::unique_lock lk{ audioSpecMutex };
		audioSpecCv.wait(lk, [&] {
			return state.exited || audioSpecReady.load() || vs.audioq.abortRequest.load();
		});
		return !state.exited && !vs.audioq.abortRequest.load();
	};

	if (!waitForAudioSpec())
	{
		return;
	}

	while (true)
	{
		Vision::CodedFrame pkt;
		int tag = 0;
		if (vs.audioq.Get(pkt, &tag, true) < 0)
			break;
		if (!waitForAudioSpec())
			break;
		if (!audioDecoder)
			continue;

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

				uint32_t firstSampleOffset = 0;
				if (!PrepareAudioFrameAfterSeek(resampled, tag, firstSampleOffset))
				{
					continue;
				}

				AudioFrameSlot slot;
				slot.picture = std::move(resampled);
				slot.packetSerial = tag;
				slot.firstSampleOffset = firstSampleOffset;

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
			playbackAudioDecoderDrained.store(true, std::memory_order_release);
			playbackAudioDecodeBusy.fetch_sub(1);
			continue;
		}

		playbackAudioDecoderDrained.store(false, std::memory_order_release);

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
// SubtitleDecodeThreadPlayback — text subtitle cue decode
// ---------------------------------------------------------------------------
void VideoPlayerContext::SubtitleDecodeThreadPlayback()
{
	while (true)
	{
		Vision::CodedFrame pkt;
		int tag = 0;
		if (vs.subtitleq.Get(pkt, &tag, true) < 0)
		{
			break;
		}
		if (!subtitleDecoder)
		{
			continue;
		}

		playbackSubtitleDecodeBusy.fetch_add(1);
		const int currentSerial = vs.subtitleq.SerialSnapshot();
		if (tag != currentSerial)
		{
			subtitleDecoder->Flush();
			playbackSubtitleDecodeBusy.fetch_sub(1);
			continue;
		}

		if (pkt && subtitlePreviewEnabled.load(std::memory_order_acquire))
		{
			Vision::SubtitleCue cue;
			auto *ffCodec = subtitleDecoder.InterpretAs<Vision::FFCodec>();
			CodecError ret = ffCodec ? ffCodec->DecodeSubtitleCue(pkt, cue) : CodecError::ExternalFailed;
			if (ret == CodecError::Success && cue)
			{
				std::lock_guard lock{ subtitleCueMutex };
				subtitleCues.emplace_back(std::move(cue));
				while (subtitleCues.size() > 64)
				{
					subtitleCues.pop_front();
				}
			}
		}

		playbackSubtitleDecodeBusy.fetch_sub(1);
	}
}

// ---------------------------------------------------------------------------
// HandleSeekInReadThread
// ---------------------------------------------------------------------------
void VideoPlayerContext::HandleSeekInReadThread()
{
	MediaType seekType = MediaType::Video;
	int64_t seekPos = 0;
	int64_t seekMin = 0;
	int64_t seekMax = 0;
	uint64_t seekSerial = 0;
	{
		std::lock_guard lock{ playbackSeekMutex };
		if (!vs.seekReq.load(std::memory_order_acquire))
		{
			return;
		}
		seekType = vs.seekStreamType;
		seekPos = vs.seekPos;
		seekMin = vs.seekMin;
		seekMax = vs.seekMax;
		seekSerial = playbackSeekRequestSerial.load(std::memory_order_relaxed);
	}

	playbackPictureCapCv.notify_all();
	playbackAudioCapCv.notify_all();

	vs.videoq.Flush();
	vs.audioq.Flush();
	vs.subtitleq.Flush();

	while (playbackVideoDecodeBusy.load() > 0 ||
	       playbackAudioDecodeBusy.load() > 0 ||
	       playbackSubtitleDecodeBusy.load() > 0)
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
			lastAudioTimestamp = INT64_MIN;
		}
		audioSize = 0;

		// swresample may retain delay-line samples from before the seek. Rebuild
		// it now that the decoder is quiescent so no pre-seek audio can leak into
		// the first frame at the new position.
		if (IsValidAudioSpec(audioOutputSpec))
		{
			SetAudioOutputSpec(audioOutputSpec);
		}
		else
		{
			std::lock_guard lock{ sampleConverterMutex };
			sampleConverter.Reset();
		}
	}
	if (subtitleDecoder)
	{
		subtitleDecoder->Flush();
		std::lock_guard lock{ subtitleCueMutex };
		subtitleCues.clear();
	}

	vs.audioClock       = NAN;
	vs.audioClockSerial = -1;
	vs.eof.store(false);
	playbackVideoDecoderDrained.store(false, std::memory_order_release);
	playbackAudioDecoderDrained.store(false, std::memory_order_release);
	lastDequeuedAudioPacketSerial = -1;
	outputAudioPacketSerial       = -1;

	InitClock(vs.audclk, &vs.audioq.serial);
	InitClock(vs.vidclk, &vs.videoq.serial);
	InitClock(vs.extclk, &vs.audioq.serial);

	CodecError seekResult = demuxer->Seek(seekType, seekPos, seekMin, seekMax);
	if (seekResult == CodecError::Success)
	{
		SetAudioSeekTarget(seekType, seekPos, vs.audioq.SerialSnapshot());
	}
	else
	{
		audioSeekTargetSerial.store(-1, std::memory_order_release);
		CLOG_ERROR("Failed to apply media seek to pts {}", seekPos);
	}
	playbackAppliedSeekSerial.store(seekSerial, std::memory_order_release);
	{
		std::lock_guard lock{ playbackSeekMutex };
		if (playbackSeekRequestSerial.load(std::memory_order_relaxed) == seekSerial)
		{
			vs.seekReq.store(false, std::memory_order_release);
		}
	}

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
		int numChannel = GetAudioOutputChannelCount();
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
	auto notifyCapacity = [=, this] {
		condition.notify_one();
	};
	if (videoThreadPool)
	{
		videoThreadPool->OnNotify(notifyCapacity);
	}
	if (audioThreadPool)
	{
		audioThreadPool->OnNotify(notifyCapacity);
	}
	if (subtitleThreadPool)
	{
		subtitleThreadPool->OnNotify(notifyCapacity);
	}

	task = [=, this]() {
		auto canReadPacket = [this] {
			const uint32_t videoTaskSize = videoThreadPool ? videoThreadPool->TaskSize().load(std::memory_order_acquire) : 0;
			const uint32_t audioTaskSize = audioThreadPool ? audioThreadPool->TaskSize().load(std::memory_order_acquire) : 0;
			const uint32_t subtitleTaskSize = subtitleThreadPool ? subtitleThreadPool->TaskSize().load(std::memory_order_acquire) : 0;
			const int rawComputeTaskSize = asyncComputeTaskCount.load(std::memory_order_acquire);
			const uint32_t computeTaskSize = rawComputeTaskSize > 0 ? (uint32_t)rawComputeTaskSize : 0;
			const uint32_t cacheSize = std::max<uint32_t>(1, (uint32_t)kCacheSize);
			return state.exited || vs.eof.load() ||
			       videoTaskSize + audioTaskSize + subtitleTaskSize + computeTaskSize < cacheSize;
		};
		while (true)
		{
			std::unique_lock lock{ mutex.demux };
			while (!condition.wait_for(lock, std::chrono::seconds{5}, canReadPacket))
			{
				CLOG_WARN(
					"VideoPlayer transcode stalled before demux: videoTasks={} audioTasks={} subtitleTasks={} asyncCompute={} cacheSize={}",
					videoThreadPool ? videoThreadPool->TaskSize().load(std::memory_order_acquire) : 0,
					audioThreadPool ? audioThreadPool->TaskSize().load(std::memory_order_acquire) : 0,
					subtitleThreadPool ? subtitleThreadPool->TaskSize().load(std::memory_order_acquire) : 0,
					asyncComputeTaskCount.load(std::memory_order_acquire),
					std::max(1, kCacheSize));
			}

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
				break;
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
uint64_t VideoPlayerContext::Seek(MediaType type, int64_t pts, int64_t min, int64_t max)
{
	if (mode == VideoPlayerMode::Playing)
	{
		uint64_t requestSerial = 0;
		{
			std::lock_guard lock{ playbackSeekMutex };
			vs.seekStreamType = type;
			vs.seekPos        = pts;
			vs.seekMin        = min;
			vs.seekMax        = max;
			requestSerial = playbackSeekRequestSerial.fetch_add(1, std::memory_order_relaxed) + 1;
			vs.seekReq.store(true, std::memory_order_release);

			// Interrupt queue waits immediately. The read thread may itself be
			// blocked in PutIfSerial(), so waiting for HandleSeekInReadThread() to
			// advance the queue serial would deadlock the seek request.
			vs.videoq.Flush();
			vs.audioq.Flush();
			vs.subtitleq.Flush();
		}
		playbackPictureCapCv.notify_all();
		playbackAudioCapCv.notify_all();
		return requestSerial;
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

	return 0;
}

// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------
VideoPlayerContext::~VideoPlayerContext()
{
	state.exited.store(true, std::memory_order_release);

	vs.videoq.Abort();
	vs.audioq.Abort();
	vs.subtitleq.Abort();
	playbackPictureCapCv.notify_all();
	playbackAudioCapCv.notify_all();
	audioSpecCv.notify_all();

    condition.notify_all();

    demuxerThread = {};
	videoDecodeThread = {};
	audioDecodeThread = {};
	subtitleDecodeThread = {};

	if (videoThreadPool)
	{
		videoThreadPool->RemoveTasks();
	}
	if (audioThreadPool)
	{
		audioThreadPool->RemoveTasks();
	}
	if (subtitleThreadPool)
	{
		subtitleThreadPool->RemoveTasks();
	}

	if (videoThreadPool)
	{
		videoThreadPool->Join();
	}
	if (audioThreadPool)
	{
		audioThreadPool->Join();
	}
	if (subtitleThreadPool)
	{
		subtitleThreadPool->Join();
	}

	// Decoders can leave a small number of reference-counted frames in the
	// playback queues when Abort() wakes their threads. Release those frames
	// explicitly while the codec objects that produced them are still alive.
	ClearPictures(pictures, picture, pictureSize);
	ClearPictures(audioFramesLegacy, audioFrame, audioSize);
	audioFrameSlots.clear();
	subtitles.clear();
	{
		std::lock_guard lock{ outputAudioMutex };
		outputAudioFrame = {};
		unconsumedSamples = 0;
	}
	{
		std::lock_guard lock{ subtitleCueMutex };
		subtitleCues.clear();
	}

	if (audioStream)
	{
		// No decoder can replace audioStream after the joins above. Quiesce the
		// callback before releasing the final device reference.
		audioStream->Stop();
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
			int numChannel = GetAudioOutputChannelCount();
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

		const uint32_t firstSampleOffset = std::min(
			slot.firstSampleOffset,
			outputAudioFrame.GetWidth());
		unconsumedSamples = outputAudioFrame.GetWidth() - firstSampleOffset;
		if (unconsumedSamples == 0)
		{
			outputAudioFrame = {};
			continue;
		}
		lastAudioTimestamp = outputAudioFrame.GetTimestamp();

		Rational tb = outputAudioFrame.GetTimebase();
		int sampleRate = outputAudioFrame.GetSampleRate();
		if (sampleRate <= 0)
			sampleRate = 48000;

		if (outputAudioFrame.GetTimestamp() != INT64_MIN && tb.denominator != 0)
		{
			double pts = (double)outputAudioFrame.GetTimestamp() * tb.Normalize();
			vs.audioClock = pts + (double)unconsumedSamples / (double)sampleRate;
			vs.audioClockSerial = slot.packetSerial;
		}
		else
		{
			vs.audioClock = NAN;
		}

		int numChannel = GetAudioOutputChannelCount();
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

        int numChannel = GetAudioOutputChannelCount();

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
	// Drop all externally cached decoder frames before destroying the player
	// and its codec contexts. In particular, `recent` commonly holds the last
	// two decoded pictures at shutdown.
	currentPicture = {};
	for (Picture &cached : timelineFrameWindow.recent)
	{
		cached = {};
	}
	timelineFrameWindow.recentCursor = 0;
	timelineFrameWindow.ClearStep();

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
	if (seekRequestActive && !player->IsSeekApplied(videoSeekRequestSerial))
	{
		return {};
	}

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
	bool selectedPictureAlreadyPopped = false;

	int64_t  picTimestamp = picture.GetTimestamp();
	Rational picTimebase  = picture.GetTimebase();
	double   picPts       = (picTimebase.denominator != 0) ? (double)picTimestamp * picTimebase.Normalize() : NAN;
	if (seekRequestActive)
	{
		Picture closestBeforeTarget;
		while (picture && ComparePicturePts(picture, videoSeekTargetPts, videoSeekTargetTimebase) < 0)
		{
			closestBeforeTarget = picture;
			if (!videoSeekForceDecode)
			{
				CacheTimelinePicture(this, picture);
				CacheTimelineStepPicture(this, TimelineStepCacheSide::Backward, picture);
			}
			player->PopPicture();
			picture = player->GetPicture();
			if (!picture)
			{
				if (closestBeforeTarget && player->IsPlaybackDrained(MediaType::Video))
				{
					picture = closestBeforeTarget;
					picTimestamp = picture.GetTimestamp();
					picTimebase = picture.GetTimebase();
					picPts = picTimebase.denominator != 0
						? (double)picTimestamp * picTimebase.Normalize()
						: NAN;
					targetReached = true;
					selectedPictureAlreadyPopped = true;
					break;
				}
				return {};
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
		videoSeekRequestSerial = 0;
		videoSeekForceDecode = false;
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
		if (!selectedPictureAlreadyPopped)
		{
			player->PopPicture();
		}
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

		if (!selectedPictureAlreadyPopped)
		{
			player->PopPicture();
		}
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
	// Once demux reaches EOF, every remaining queued picture is part of the
	// visible tail. Dropping late frames here makes playback jump from the last
	// presented timestamp straight to the declared duration.
	bool canDrop = !vs.eof.load(std::memory_order_acquire) &&
		(framedrop > 0 || (framedrop < 0 && vs.avSyncType != FfplayAvSyncType::VideoMaster));
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
    (void)player->Seek(type, pts, min, max);
}

void VideoPlayerComponent::SeekToFrame(MediaType type, int64_t pts, bool forceDecode)
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
			videoSeekRequestSerial = 0;
			videoSeekForceDecode = false;
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
		if (!forceDecode)
		{
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
		}

		const int64_t distance = EstimateTimelineFrameDistance(currentPicture, pts, targetTb, animator);
		const bool discontinuousSeek =
		    distance == std::numeric_limits<int64_t>::max() || distance > TIMELINE_STEP_NEAR_FRAMES;

		if (forceDecode)
		{
			// A forced demux seek never consumes timeline caches. Drop them before
			// flushing the decoder so old hardware frames cannot pin decode surfaces.
			ClearTimelineCaches(this);
		}
		else if (discontinuousSeek)
		{
			ClearTimelineStepCaches(this);
		}

		if (!forceDecode && currentCmp != 0)
		{
			CacheTimelinePicture(this, currentPicture);
			if (!discontinuousSeek)
			{
				MoveCurrentToTimelineStepCache(this, seekDirection < 0);
			}
		}

		videoSeekTargetPts = pts;
		videoSeekTargetTimebase = targetTb;
		videoSeekForceDecode = forceDecode;
		videoFrameTimerInit = false;
		timelinePinnedPicturePts = std::numeric_limits<int64_t>::min();
		timelinePinnedPictureTimebase = {};
		timelineSeekDirection = seekDirection;
		currentPicture = {};
	}
	uint64_t requestSerial = player->Seek(
		type,
		pts,
		std::numeric_limits<int64_t>::min(),
		std::numeric_limits<int64_t>::max());
	if (type == MediaType::Video)
	{
		videoSeekRequestSerial = requestSerial;
	}
}

bool VideoPlayerComponent::IsEof() const
{
	return player->IsEof();
}

bool VideoPlayerComponent::IsPlaybackDrained(MediaType type) const
{
    return player && player->IsPlaybackDrained(type);
}

bool VideoPlayerComponent::HasPendingSeek() const
{
	return player && player->HasPendingSeek();
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

void VideoPlayerComponent::SetSubtitlePreviewEnabled(bool enabled)
{
	if (player)
	{
		player->SetSubtitlePreviewEnabled(enabled);
	}
}

bool VideoPlayerComponent::IsSubtitlePreviewEnabled() const
{
	return player ? player->subtitlePreviewEnabled.load(std::memory_order_acquire) : false;
}

Vision::SubtitleCue VideoPlayerComponent::GetCurrentSubtitleCue(double seconds)
{
	return player ? player->GetCurrentSubtitleCue(seconds) : Vision::SubtitleCue{};
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
	WaitUntilFinished();
}

void VideoPlayerComponent::WaitUntilFinished()
{
	if (player)
	{
		player->Join();
	}
}

bool VideoPlayerComponent::operator!()
{
	return !player;
}

int64_t VideoPlayerComponent::GetLastAudioTimestamp() const
{
	return player->lastAudioTimestamp.load(std::memory_order_acquire);
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

	// FFCodec converts decoded audio timestamps to sample units in
	// GetPicture(). The codec time base still describes the input packets; for
	// MP3 it can be 1 / (sampleRate * 320), which makes the UI playhead advance
	// 320 times too slowly if paired with a decoded-frame timestamp.
	const Vision::AudioFormatSpec audioFormat = decoder->GetAudioFormat();
	if (audioFormat.sampleRate > 0)
	{
		return { 1, audioFormat.sampleRate };
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
