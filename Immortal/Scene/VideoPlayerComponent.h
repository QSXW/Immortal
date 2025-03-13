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

class VideoPlayerContext;
struct VideoPlayerComponent : public IObject, public Component
{
    SL_SWAPPABLE(VideoPlayerComponent)

    DEFINE_COMPONENT_TYPE(VideoPlayer)

    VideoPlayerComponent();

    VideoPlayerComponent(const String &path, int cacheSize = 3, const Vision::DecodingPreference &preference = Vision::DecodingPreference::Auto, const VideoDecodeCallbacks &callbacks = {});

    VideoPlayerComponent(Ref<Demuxer> demuxer, Ref<VideoCodec> decoder, Ref<VideoCodec> audioDecoder = nullptr, Ref<VideoCodec> subtitleDecoder = nullptr);

    ~VideoPlayerComponent();

    void StartPlay();

    Picture GetLivePicture();

    Picture GetPicture();

    Picture GetAudioFrame();

    void PopPicture();

    void PopAudioFrame();

    void Seek(double seconds, int64_t min, int64_t max);

    bool IsEof() const;

    void Swap(VideoPlayerComponent &other);

    Animator *GetAnimator() const;

    const String &GetSource() const;

    const Vision::DisplayOrientation *GetDisplayOrientation() const;

    void EnumerateTracks(MediaType mediaType, std::vector<Vision::TrackInfo> &tracks);

    void OnPause(bool enabled);

    CodecError SwitchTrack(MediaType mediaType, int index);

    bool HasStream(MediaType type) const;

    bool operator !();

    bool pause = false;

public:
    URef<VideoPlayerContext> player;

    bool startPlay = false;

    Picture currentPicture;
};

}
