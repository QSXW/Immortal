#include "VideoOutputComponent.h"
#include "Vision/Mux/ImageMuxer.h"

namespace Immortal
{

#define IMMORTAL_HAVE_VIDEO_PLAYER_STATISTIC 1

int CompareTimestamp(int64_t timestampA, Rational timebaseA, int64_t timestampB, Rational timebaseB)
{
	int64_t a = timebaseA.numerator * (int64_t) timebaseB.denominator;
	int64_t b = timebaseB.numerator * (int64_t) timebaseA.denominator;

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

public:
	void SetFilterGraph(const std::shared_ptr<FilterGraphComponent> &graph)
	{
		filterGraph = graph;
	}

protected:
	LightArray<Codec *, 4> codecs;

	Ref<Codec> videoEncoder;

	Ref<Codec> audioEncoder;

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

	std::shared_ptr<FilterGraphComponent> filterGraph;
};

VideoOutput::VideoOutput(const String &filepath, const EncodeInfo *pEncodeInfo, uint32_t numEncodeInfo) :
    codecs{},
    videoEncoder{},
    audioEncoder{},
    muxer{},
    mediaTypes{},
    videoEncodeThread{1},
    audioEncodeThread{1},
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

	bool image = FileSystem::IsImage(filepath);
	ImageEncodeInfo imageEncodeInfo = {
	    .quality = 100
	};

	EncodeInfo videoEncodeInfo = {};
	for (uint32_t i = 0; i < numEncodeInfo; i++)
	{
		auto &encodeInfo = pEncodeInfo[i];
		switch (encodeInfo.mediaType)
		{
			case MediaType::Video:
				videoEncodeInfo = encodeInfo;
				videoEncoder = image ? Vision::SelectSuitableCodec(filepath, false, imageEncodeInfo) : new Vision::FFCodec{encodeInfo};
				codecs[i] = videoEncoder.Get();
				break;

			case MediaType::Audio:
				if (!image)
				{
					audioEncoder = new Vision::FFCodec{encodeInfo};
					codecs[i] = audioEncoder.Get();
				}
				break;
			default:
				break;
		}
	}

	if (image)
	{
		muxer = new Vision::ImageMuxer;
	}
	else
	{
		muxer = new Vision::FFDemuxer;
	}

	if (muxer->Open(filepath, codecs.data(), pEncodeInfo, numEncodeInfo) != CodecError::Success)
	{
		muxer.Reset();
		return;
	}

	muxThread = std::move(Thread{[=, this] {
		int64_t pts = 0;
		int64_t audioSamples = 0;

		Rational videoTimebase;
		Rational framerate;
		Rational timebase;
		if (videoEncoder)
		{
			videoTimebase = videoEncodeInfo.timeBase;
			framerate     = videoEncodeInfo.framerate;
			timebase      = videoTimebase * framerate;
		}

		Rational audioTimebase{};
		if (audioEncoder)
		{
			audioTimebase = audioEncoder.InterpretAs<Vision::FFCodec>()->GetTimebase();
		}

		CodedFrame videoFrame;
		CodedFrame audioFrame;
		while (true)
		{
			bool video = (!audioEncoder ||
			             (CompareTimestamp(pts, videoTimebase, audioSamples, audioTimebase) <= 0) || audioQueue.empty());
			if (frameInQueue && videoEncoder && video)
			{
				if (videoQueue.try_dequeue(videoFrame))
				{
					pts = videoFrame.GetTimestamp();
					muxer->Write(videoFrame, 0);
#if IMMORTAL_HAVE_VIDEO_PLAYER_STATISTIC
					//              if (callbacks.progressListener)
					//              {
					// callbacks.progressListener->SetProgress(frames / double(duration));
					//              }
					if (callbacks.ReportProgress)
					{
						callbacks.ReportProgress(frames, pts, timebase);
					}
#endif
					frameInQueue--;
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
			if ((!videoEncoder || (videoFinished && videoQueue.empty() && videoEncodeThread.TaskSize() == 0)) &&
			    (!audioEncoder || (audioFinished && audioQueue.empty() && audioEncodeThread.TaskSize() == 0)))
			{
#ifdef IMMORTAL_HAVE_VIDEO_PLAYER_STATISTIC
				auto time = timer.Duration();
				LOG::INFO("Encoding Statistic: frames:{}, time:{}, fps:{}", frames, time, frames / time);
#endif
				break;
			}
		}
		muxer->Close();
	}});

	muxThread.SetDescription("Mux");

#ifdef IMMORTAL_HAVE_VIDEO_PLAYER_STATISTIC
	timer.Start();
#endif
}

VideoOutput::~VideoOutput()
{
	muxThread.Join();
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
				frameInQueue++;
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
		if (codedFrame)
		{
			frameInQueue++;
			videoQueue.enqueue(std::move(codedFrame));
		}
		while (codedFrame = videoEncoder->GetCodedFrame())
		{
			frameInQueue++;
			videoQueue.enqueue(std::move(codedFrame));
		}
	});

	videoEncodeThread.OnNotify([=, this] {
		if (!Blocking())
		{
			blocked = false;
			blocked.notify_one();
		}
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
		case MediaType::Video: {
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
		case MediaType::Audio: {
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
		case MediaType::Subtitle: {
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
	videoEncodeThread.Join();
	audioEncodeThread.Join();
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

bool VideoOutput::operator!() const
{
	return !muxer;
}

VideoOutputComponent::VideoOutputComponent() :
    v{}
{

}

VideoOutputComponent::VideoOutputComponent(const String &filepath, const EncodeInfo *pEncodeInfo, uint32_t numEncodeInfo) :
    v{new VideoOutput{filepath, pEncodeInfo, numEncodeInfo}}
{

}

VideoOutputComponent::~VideoOutputComponent()
{
	v.Reset();
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

void VideoOutputComponent::SetFilterGraph(const std::shared_ptr<FilterGraphComponent> &graph)
{
	v->SetFilterGraph(graph);
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

void VideoOutputComponent::Swap(VideoOutputComponent &other)
{
	v.Swap(other.v);
}

}
