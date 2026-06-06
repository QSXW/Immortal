/**
 * Copyright (C) 2024, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#pragma once

#include "Component.h"

namespace Immortal
{

class VideoPlayerContext;
struct VideoPlayerComponent : public Component
{
    SL_SWAPPABLE(VideoPlayerComponent)

    DEFINE_COMPONENT_TYPE(VideoPlayer)

    VideoPlayerComponent();

    VideoPlayerComponent(const String &path);

    VideoPlayerComponent(Ref<Demuxer> demuxer, Ref<VideoCodec> decoder, Ref<VideoCodec> audioDecoder = nullptr);

    ~VideoPlayerComponent();

    Picture GetPicture();

    Picture GetAudioFrame();

    void PopPicture();

    void PopAudioFrame();

    void Seek(double seconds, int64_t min, int64_t max);

    void Swap(VideoPlayerComponent &other);

    Animator *GetAnimator() const;

    const String &GetSource() const;

    const Vision::DisplayOrientation *GetDisplayOrientation() const;

public:
    URef<VideoPlayerContext> player;
};

}
