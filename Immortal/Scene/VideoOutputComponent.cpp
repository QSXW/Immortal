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

class VideoOutput : public IObject
{
public:
	VideoOutput(const String &filepath, const EncodeInfo *pEncodeInfo, uint32_t numEncodeInfo);

    ~VideoOutput();

	void EnqueueVideoFrame(Picture &&picture);

	void EnqueueAudioFrame(Picture &&picture);

    CodecError Send(const Picture &picture, int stream);

	CodecError Write(const CodedFrame &codedFrame, int stream);

	void Close();

	void Bind(const VideoEncodeCallbacks &value);

	bool Blocking() const;

    bool operator!() const;

protected:
	LightArray<Codec *, 4> codecs;

	URef<Codec> videoEncoder;

	URef<Codec> audioEncoder;

	std::vector<MediaType> mediaTypes;

	URef<Demuxer> muxer;

	ConcurrentQueue<Vision::CodedFrame> videoQueue;

	ConcurrentQueue<Vision::CodedFrame> audioQueue;

	ThreadPool videoEncodeThread;
	ThreadPool audioEncodeThread;
	Thread muxThread;

	std::atomic_bool blocked = false;

	std::atomic_bool videoFinished;

	std::atomic_bool audioFinished;

	int64_t timestamp;

	int64_t audioTimestamp;

	std::atomic_bool waiting;

	std::condition_variable condition;

	Timer timer;

	int frames;

	int64_t duration = 0;

	VideoEncodeCallbacks callbacks;

    std::atomic<int> frameInQueue;
};

VideoOutput::VideoOutput(const String &filepath, const EncodeInfo *pEncodeInfo, uint32_t numEncodeInfo) :
    codecs{},
    videoEncoder{},
    audioEncoder{},
    muxer{},
    mediaTypes{},
    videoEncodeThread{ 1 },
    audioEncodeThread{ 1 },
    muxThread{},
    timestamp{},
    audioTimestamp{},
    videoFinished{},
    audioFinished{},
    timer{},
    frames{},
    frameInQueue{}
{
	codecs.resize(numEncodeInfo);
	for (uint32_t i = 0; i < numEncodeInfo; i++)
    {
		auto &encodeInfo = pEncodeInfo[i];
		codecs[i] = new Vision::FFCodec{encodeInfo};
    }

    muxer = new Vision::FFDemuxer;
	if (muxer->Open(filepath, codecs.data(), numEncodeInfo) != CodecError::Success)
    {
		muxer.Reset();
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


VideoOutput::~VideoOutput()
{
    for (size_t i = 0; i < codecs.size(); i++)
    {
		delete codecs[i];
    }
}

void VideoOutput::EnqueueVideoFrame(Picture &&_picture)
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

void VideoOutput::EnqueueAudioFrame(Picture &&_picture)
{
	if (!audioEncoder)
	{
		return;
	}

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

CodecError VideoOutput::Send(const Picture &picture, int stream)
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

CodecError VideoOutput::Write(const CodedFrame &codedFrame, int stream)
{
    return muxer->Write(codedFrame, stream);
}

void VideoOutput::Close()
{
    muxThread.Join();
}

void VideoOutput::Bind(const VideoEncodeCallbacks &value)
{
	callbacks = value;
}

bool VideoOutput::Blocking() const
{
	return videoEncodeThread.TaskSize() > 3;
}

bool VideoOutput::operator !() const
{
	return !muxer;
}

VideoOutputComponent::VideoOutputComponent(const String &filepath, const EncodeInfo *pEncodeInfo, uint32_t numEncodeInfo) :
    v{new VideoOutput{filepath, pEncodeInfo, numEncodeInfo}}
{

}

void VideoOutputComponent::EnqueueVideoFrame(Picture &&picture)
{
	v->EnqueueVideoFrame(std::move(picture));
}

void VideoOutputComponent::EnqueueAudioFrame(Picture &&picture)
{
	v->EnqueueAudioFrame(std::move(picture));
}

CodecError VideoOutputComponent::Write(const CodedFrame &codedFrame, int stream)
{
	return v->Write(codedFrame, stream);
}

void VideoOutputComponent::Join()
{
	v->Close();
}

void VideoOutputComponent::Close()
{
	v->Close();
}

void VideoOutputComponent::Bind(const VideoEncodeCallbacks &value)
{
	v->Bind(value);
}

bool VideoOutputComponent::Blocking() const
{
	return v->Blocking();
}

VideoOutputComponent::operator bool() const
{
	return !!(*v);
}

}
