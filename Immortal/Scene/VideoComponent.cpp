/**
 * Copyright (C) 2022, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#include "Component.h"

namespace Immortal
{

#define forever for (;;)

VideoPlayerContext::VideoPlayerContext(Ref<Vision::VideoCodec> decoder, Ref<Vision::Interface::Demuxer> demuxer) :
	demuxerThread{},
	mutex{},
	decoder{decoder},
	demuxer{demuxer},
	lastPicture{},
	state{}
{
	demuxerThread = new Thread{[=, this]() {
		while (!state.exited)
		{
			Vision::CodedFrame codedFrame;
			while (!state.exited && (pictures.size() < 7 && pictureCodedFrames.size() < 14) && !demuxer->Read(&codedFrame))
			{
				std::unique_lock lock{mutex};
				pictureCodedFrames.push(codedFrame);
			}
		}
	}};

	videoDecodeThread = new Thread{
	    [=, this]() {
		    while (!state.exited)
			{
			    while (!state.exited && pictures.size() < 7 && !pictureCodedFrames.empty())
				{
				    Vision::CodedFrame codedFrame;
				    {
					    std::unique_lock lock{mutex};
					    codedFrame = pictureCodedFrames.front();
					    pictureCodedFrames.pop();
				    }
				    if (decoder->Decode(codedFrame) == CodecError::Succeed)
				    {
					    Vision::Picture picture = decoder->GetPicture();

					    std::unique_lock lock{mutex};
					    pictures.push(picture);
				    }
				}
		    }
	    }};

	demuxerThread->Start();
	videoDecodeThread->Start();
}

VideoPlayerContext::~VideoPlayerContext()
{
	state.exited = true;
	demuxerThread.Reset();
	videoDecodeThread.Reset();
}

VideoPlayerComponent::VideoPlayerComponent(Ref<Vision::VideoCodec> decoder, Ref<Vision::Interface::Demuxer> demuxer) :
    player{new VideoPlayerContext{decoder, demuxer}}
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

void VideoPlayerComponent::PopPicture()
{
	player->PopPicture();
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
