#include "VideoOutputComponent.h"

namespace Immortal
{

#define IMMORTAL_HAVE_VIDEO_PLAYER_STATISTIC 1

int CompareTimestamp(int64_t timestampA, Rational timebaseA, int64_t timestampB, Rational timebaseB)
{
    int64_t a = timebaseA.numerator * (int64_t)timebaseB.denominator;
    int64_t b = timebaseB.numerator * (int64_t)timebaseA.denominator;

    return (timestampA * a > timestampB * b) - (timestampA * a < timestampB * b);
}

VideoOutputComponent::VideoOutputComponent(const String &filepath, const EncodeInfo &videoEncodeInfo, const EncodeInfo &audioEncodeInfo, const std::initializer_list<MediaType> &&streamInfos) :
    videoEncoder{ new Vision::FFCodec{ videoEncodeInfo } },
    audioEncoder{},
    muxer{},
    mediaTypes{ streamInfos },
    videoEncodeThread{ 1 },
    audioEncodeThread{ 1 },
    muxThread{},
    timestamp{},
    audioTimestamp{},
    videoFinished{},
    audioFinished{},
    timer{},
    frames{}
{
    if (audioEncodeInfo.codecId != CodecId::None)
    {
        audioEncoder = new Vision::FFCodec{ audioEncodeInfo };
    }

    muxer = new Vision::FFDemuxer;
    if (muxer->Open(filepath, videoEncoder, audioEncoder, nullptr, std::move(streamInfos)) != CodecError::Success)
    {
        return;
    }

    muxThread = std::move(Thread{ [=, this] {
        int64_t pts = 0;
        int64_t audioSamples = 0;

        Rational videoTimebase = videoEncoder.InterpretAs<Vision::FFCodec>()->GetTimebase();
        Rational framerate     = videoEncoder.InterpretAs<Vision::FFCodec>()->GetFramerate();
        Rational timebase      = videoTimebase * framerate;
        Rational audioTimebase{};
        if (audioEncoder)
        {
            audioTimebase = audioEncoder.InterpretAs<Vision::FFCodec>()->GetTimebase();
        }

        CodedFrame videoFrame;
        CodedFrame audioFrame;
        while (true)
        {
            if (videoEncoder && (!audioEncoder ||
                (CompareTimestamp(pts, videoTimebase, audioSamples, audioTimebase) <= 0)))
            {
                if (videoQueue.try_dequeue(videoFrame))
                {
                    pts = videoFrame.GetTimestamp();
                    muxer->Write(videoFrame, 0);
#if IMMORTAL_HAVE_VIDEO_PLAYER_STATISTIC
                    if (callbacks.ReportProgress)
                    {
                        callbacks.ReportProgress(frames, pts, timebase);
                    }
#endif
                }
            }
            else
            {
                if (audioQueue.try_dequeue(audioFrame))
                {
                    audioSamples = audioFrame.GetTimestamp();
                    muxer->Write(audioFrame, 1);
                }
            }
            if (videoFinished &&
                videoEncodeThread.TaskSize() == 0 &&
                (!audioEncoder || (audioFinished && audioEncodeThread.TaskSize() == 0)))
            {
#ifdef IMMORTAL_HAVE_VIDEO_PLAYER_STATISTIC
                auto time = timer.Duration();
                LOG::INFO("Encoding Statistic: frames:{}, time:{}, fps:{}", frames, time, frames / time);
#endif
                break;
            }
        }
        muxer->Close();
    } });

    muxThread.Start();
    muxThread.SetDescription("Mux");

#ifdef IMMORTAL_HAVE_VIDEO_PLAYER_STATISTIC
    timer.Start();
#endif
}

void VideoOutputComponent::EnqueueVideoFrame(Picture &&_picture)
{
    if (Blocking())
    {
        blocked = true;
        blocked.wait(true);
    }

    videoEncodeThread.Enqueue([=, this, picture = std::move(_picture)] {
        picture.SetTimestamp(timestamp++);
        if (picture.GetFlags() & Vision::PictureFlags::Eof)
        {
            videoEncoder->Flush();
            CodedFrame codedFrame{};
            while (codedFrame = videoEncoder->GetCodedFrame())
            {
                videoQueue.enqueue(std::move(codedFrame));
            }
            videoFinished = true;
            return;
        }

        CodedFrame codedFrame{};
        CodecError ret = videoEncoder->Encode(picture, codedFrame);
#if IMMORTAL_HAVE_VIDEO_PLAYER_STATISTIC
        frames++;
#endif
        if (ret != CodecError::Success)
        {
            LOG::ERR("Failed to encode video frame!");
            videoFinished = true;
        }

        while (codedFrame = videoEncoder->GetCodedFrame())
        {
            videoQueue.enqueue(std::move(codedFrame));
        }
    });

    videoEncodeThread.OnNotify([=, this] {
        blocked = false;
		blocked.notify_one();
    });
}

void VideoOutputComponent::EnqueueAudioFrame(Picture &&_picture)
{
    audioEncodeThread.Enqueue([=, this, picture = std::move(_picture)] {
        if (picture.GetFlags() & Vision::PictureFlags::Eof)
        {
            audioEncoder->Flush();
            CodedFrame codedFrame{};
            while (codedFrame = audioEncoder->GetCodedFrame())
            {
                audioQueue.enqueue(std::move(codedFrame));
            }
            audioFinished = true;
            return;
        }

        picture.SetTimestamp(audioTimestamp);
        audioTimestamp += picture.GetWidth();

        CodedFrame codedFrame{};
        if (audioEncoder->Encode(picture, codedFrame) != CodecError::Success)
        {
            LOG::ERR("Failed to encode audio frame!");
            audioFinished = true;
        }

        while (codedFrame = audioEncoder->GetCodedFrame())
        {
            audioQueue.enqueue(std::move(codedFrame));
        }
    });
}

CodecError VideoOutputComponent::Send(const Picture &picture, int stream)
{
    CodecError ret = CodecError::Success;

    switch (mediaTypes[stream])
    {
    case MediaType::Video:
    {
        if (videoEncodeThread.TaskSize() > 7)
        {
            return CodecError::Again;
        }
        videoEncodeThread.Enqueue([=, this] {
            picture.SetTimestamp(timestamp++);
            CodedFrame encodedFrame;
            if (videoEncoder->Encode(picture, encodedFrame) == CodecError::Success)
            {
                videoQueue.enqueue(encodedFrame);
            }
            });
        break;
    }
    case MediaType::Audio:
    {
        audioEncodeThread.Enqueue([=, this] {
            picture.SetTimestamp(audioTimestamp);
            audioTimestamp += picture.GetWidth();
            CodedFrame encodedFrame;
            if (audioEncoder->Encode(picture, encodedFrame) == CodecError::Success)
            {
                audioQueue.enqueue(encodedFrame);
            }
            });
        break;
    }
    case MediaType::Subtitle:
    {
        break;
    }

    default:
        break;
    }

    return ret;
}

CodecError VideoOutputComponent::Write(const CodedFrame &codedFrame, int stream)
{
    return muxer->Write(codedFrame, stream);
}

void VideoOutputComponent::Close()
{
    muxThread.Join();
}

}
