/**
 * Copyright (C) 2022, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#include "Component.h"

namespace Immortal
{

#define forever for (;;)

VideoPlayerContext::VideoPlayerContext(Ref<Vision::Interface::Demuxer> demuxer, Ref<Vision::VideoCodec> decoder, Ref<Vision::VideoCodec> audioDecoder) :
    demuxerThread{},
    mutex{},
    decoder{decoder},
    audioDecoder{ audioDecoder },
    demuxer{demuxer},
    lastPicture{},
    state{}
{
    demuxerThread = new Thread{[=, this]() {
        while (!state.exited)
        {
            Vision::CodedFrame codedFrame;

            if (pictures.size() > 3 && audioFrames.size() != 0)
            {
                continue;
            }

            if (pictures.size() != 0 && audioFrames.size() > 7)
            {
                continue;
            }

            if (queues[0].size() > 7 && queues[1].size() != 0)
            {
                continue;
            }

            if (queues[0].size() != 0 && queues[1].size() > 7)
            {
                continue;
            }

            if (demuxer->Read(&codedFrame) != CodecError::Succeed)
            {
                continue;
            }

            if (codedFrame.Type == MediaType::Subtitle)
            {
                continue;
            }
            if (!audioDecoder && codedFrame.Type == MediaType::Audio)
            {
                continue;
            }
            auto &queue = queues[(int)codedFrame.Type];
            std::unique_lock lock{mutex};
            queue.push(codedFrame);
        }
    }};

    videoDecodeThread = new Thread{
        [=, this]() {
            while (!state.exited)
            {
                while (!state.exited && pictures.size() < 7 && !queues[0].empty())
                {
                    Vision::CodedFrame codedFrame;
                    {
                        std::unique_lock lock{mutex};
                        if (!queues[0].empty())
                        {
                            codedFrame = queues[0].front();
                            queues[0].pop();
                        }
                    }
                    if (codedFrame && decoder->Decode(codedFrame) == CodecError::Succeed)
                    {
                        Vision::Picture picture = decoder->GetPicture();

                        std::unique_lock lock{mutex};
                        pictures.push(picture);
                    }
                }
            }
        }};

    if (audioDecoder)
    {
        audioDecodeThread = new Thread{
            [=, this]() {
                while (!state.exited)
                {
                    while (!state.exited && audioFrames.size() < 7 && !queues[1].empty())
                    {
                        Vision::CodedFrame codedFrame;
                        {
                            std::unique_lock lock{mutex};
                            if (!queues[1].empty())
                            {
                                codedFrame = queues[1].front();
                                queues[1].pop();
                            }
                        }
                        if (codedFrame && audioDecoder->Decode(codedFrame) == CodecError::Succeed)
                        {
                            Vision::Picture picture = audioDecoder->GetPicture();

                            std::unique_lock lock{mutex};
                            audioFrames.push(picture);
                        }
                    }
                }
            }};

        audioDecodeThread->Start();
    }

    demuxerThread->Start();
    videoDecodeThread->Start();
}

VideoPlayerContext::~VideoPlayerContext()
{
    state.exited = true;
    demuxerThread.Reset();
    videoDecodeThread.Reset();
}

VideoPlayerComponent::VideoPlayerComponent(Ref<Vision::Interface::Demuxer> demuxer, Ref<Vision::VideoCodec> decoder, Ref<Vision::VideoCodec> audioDecoder) :
    player{new VideoPlayerContext{demuxer, decoder, audioDecoder}}
{

}

VideoPlayerComponent::~VideoPlayerComponent()
{
    player.Reset();
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
    player->PopAudioFrame();
}

Animator *VideoPlayerComponent::GetAnimator() const
{
    return player->decoder->GetAddress<Animator>();
}

const std::string &VideoPlayerComponent::GetSource() const
{
    return player->GetSource();
}

}
