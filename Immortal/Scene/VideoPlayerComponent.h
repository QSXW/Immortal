/**
 * Copyright (C) 2024, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#pragma once

#include "Component.h"
#include "Vision/Codec.h"

namespace Immortal
{

struct VideoDecodeCallbacks
{
	std::function<void(Picture &&)> VideoDecodeFinishSlot;
    std::function<void(Picture &&)> AudioDecodeFinishSlot;
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

    void StartPlay();

    Picture GetLivePicture();

    Picture GetPicture();

    Picture GetAudioFrame();

    void PopPicture();

    void PopAudioFrame();

    void Seek(MediaType type, int64_t pts, int64_t min, int64_t max);

    bool IsEof() const;

    void Swap(VideoPlayerComponent &other);

    Animator *GetAnimator(MediaType type = MediaType::Video) const;

    const String &GetSource() const;

    const Vision::DisplayOrientation *GetDisplayOrientation() const;

    void EnumerateTracks(MediaType mediaType, std::vector<Vision::TrackInfo> &tracks);

    void OnPause(bool enabled);

    CodecError SwitchTrack(MediaType mediaType, int index);

    bool HasStream(MediaType type) const;

    CodecError GetStreamInfo(MediaType type, CodecInfo &streamInfo);

    void SetFilterGraph(const std::shared_ptr<FilterGraphComponent> &graph, Format format);

    void SetCallbacks(const VideoDecodeCallbacks &callbacks);

    void SetAudioOutputSpec(const Vision::AudioFormatSpec &outputSpec);

    void Join();

    int64_t GetLastAudioTimestamp() const;

    Rational GetAudioTimebase() const;

    bool operator !();

    operator bool() const;

    void SetSpeed(double speed);

public:
    URef<VideoPlayerContext> player;

    double speed = 1.0f;

    Picture currentPicture;
};

}
