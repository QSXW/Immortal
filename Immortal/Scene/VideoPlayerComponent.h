/**
 * Copyright (C) 2024, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#pragma once

#include "Component.h"
#include "Vision/Codec.h"
#include <array>
#include <deque>
#include <limits>

namespace Immortal
{

struct VideoDecodeCallbacks
{
	std::function<void(Picture &&)> VideoDecodeFinishSlot;
    std::function<void(Picture &&)> AudioDecodeFinishSlot;
};

struct TimelineFrameWindow
{
    std::array<Picture, 8> recent{};
    size_t recentCursor = 0;
    std::deque<Picture> backward;
    std::deque<Picture> forward;

    void ClearStep()
    {
        backward.clear();
        forward.clear();
    }
};

enum class VideoPlayerMode
{
    Playing,
    Transcoding,
    MetaReading
};

enum class StreamEnabledFlags
{
    None     = 0,
	Video    = BIT(0),
	Audio    = BIT(1),
	Subtitle = BIT(2),
	/// Decode audio normally but do NOT create the system AudioStream.
	/// The owner is expected to pull samples via `VideoPlayerComponent::PullAudio()`.
	AudioMixerMode = BIT(3),
};
SL_ENABLE_BITWISE_OPERATOR(StreamEnabledFlags, uint32_t)

class FilterGraphComponent;
class VideoPlayerContext;
struct VideoPlayerComponent : public IObject, public Component
{
    SL_SWAPPABLE(VideoPlayerComponent)

    DEFINE_COMPONENT_TYPE(VideoPlayer)

    VideoPlayerComponent();

    VideoPlayerComponent(const String &path, int cacheSize = 3, const Vision::DecodingPreference &preference = Vision::DecodingPreference::Auto, VideoPlayerMode mode = VideoPlayerMode::Playing, StreamEnabledFlags flags = StreamEnabledFlags::None);

    ~VideoPlayerComponent();

    CodecError Open(const String &path, int cacheSize = 3, const Vision::DecodingPreference &preference = Vision::DecodingPreference::Auto, VideoPlayerMode mode = VideoPlayerMode::Playing, StreamEnabledFlags flags = StreamEnabledFlags::None, bool startImmediately = true);

    void StartPlay();

    void BeginPlayback();

    Picture GetLivePicture();

    Picture GetPicture();

    Picture GetAudioFrame();

    void PopPicture();

    void PopAudioFrame();

    void Seek(MediaType type, int64_t pts, int64_t min, int64_t max);

    void SeekToFrame(MediaType type, int64_t pts, bool forceDecode = false);

    bool IsEof() const;

    bool IsPlaybackDrained(MediaType type) const;

    bool HasPendingSeek() const;

    void Swap(VideoPlayerComponent &other);

    Animator *GetAnimator(MediaType type = MediaType::Video) const;

    const String &GetSource() const;

    const Vision::DisplayOrientation *GetDisplayOrientation() const;

    void EnumerateTracks(MediaType mediaType, std::vector<Vision::TrackInfo> &tracks);

    void OnPause(bool enabled);

    void SetMuted(bool enabled);

    bool IsMuted() const;

    void SetVolume(float value);

    float GetVolume() const;

    CodecError SwitchTrack(MediaType mediaType, int index);

    void SetSubtitlePreviewEnabled(bool enabled);

    bool IsSubtitlePreviewEnabled() const;

    Vision::SubtitleCue GetCurrentSubtitleCue(double seconds);

    bool HasStream(MediaType type) const;

    CodecError GetStreamInfo(MediaType type, CodecInfo &streamInfo);

    void SetFilterGraph(const std::shared_ptr<FilterGraphComponent> &graph, Format format);

    void SetCallbacks(const VideoDecodeCallbacks &callbacks);

    void SetAudioOutputSpec(const Vision::AudioFormatSpec &outputSpec);

    void Join();

    void WaitUntilFinished();

    int64_t GetLastAudioTimestamp() const;

    Rational GetAudioTimebase() const;

    bool operator !();

    operator bool() const;

    void SetSpeed(double speed);

    /// Pull mixed audio samples directly (used by Montage's `AudioMixer`).
    /// Only meaningful when the player was opened with `StreamEnabledFlags::AudioMixerMode`.
    /// Returns the number of bytes filled (may be 0 if no decoded audio is yet available).
    uint32_t PullAudio(uint8_t *data, uint32_t samples);

    /// Configure the desired output audio spec (sample rate, channels, format).
    /// Used together with `AudioMixerMode` so the resampler matches the mixer device format.
    void ConfigureMixerSpec(const Vision::AudioFormatSpec &outputSpec);

    /// Montage timeline pool mode: paused seek/step uses exact frame cache, normal playback keeps
    /// the ffplay-style display clock.
    void SetTimelineDecodeDrive(bool enable);

    URef<VideoPlayerContext> player;

    double speed = 1.0f;

    Picture currentPicture;

    double  videoFrameTimer         = 0.0;
    bool    videoFrameTimerInit     = false;
    double  videoLastDisplayedPts   = 0.0;
    int     videoLastDisplayedSerial = -1;
    int     lastVideoQueueSerialForFrameTimer = INT_MIN;
    int64_t videoSeekTargetPts = std::numeric_limits<int64_t>::min();
    Rational videoSeekTargetTimebase{};
    uint64_t videoSeekRequestSerial = 0;
    bool videoSeekForceDecode = false;
    int timelineSeekDirection = 0;
    TimelineFrameWindow timelineFrameWindow;
    int64_t timelinePinnedPicturePts = std::numeric_limits<int64_t>::min();
    Rational timelinePinnedPictureTimebase{};
};

}
