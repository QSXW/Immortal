#include "VideoOutputComponent.h"
#include "Vision/MediaFormat/FFFormat.h"
#include "Vision/MediaFormat/ImageFormat.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <deque>
#include <mutex>

namespace Immortal
{

#define IMMORTAL_HAVE_VIDEO_PLAYER_STATISTIC 1

static bool HasValidPictureTimestamp(const Picture &picture)
{
	const Rational &timebase = picture.GetTimebase();
	return timebase.numerator != 0 && timebase.denominator != 0;
}

static bool IsValidTimebase(const Rational &timebase)
{
	return timebase.numerator != 0 && timebase.denominator != 0;
}

static int CompareTimestamp(int64_t timestampA, Rational timebaseA, int64_t timestampB, Rational timebaseB)
{
	if (!IsValidTimebase(timebaseA) || !IsValidTimebase(timebaseB))
	{
		return (timestampA > timestampB) - (timestampA < timestampB);
	}

	const long double a = (long double)timestampA * (long double)timebaseA.numerator / (long double)timebaseA.denominator;
	const long double b = (long double)timestampB * (long double)timebaseB.numerator / (long double)timebaseB.denominator;
	return (a > b) - (a < b);
}

static const char *CodecErrorName(CodecError error)
{
	switch (error)
	{
		case CodecError::CorruptedBitstream:
			return "Corrupted bitstream";
		case CodecError::OutOfMemory:
			return "Out of memory";
		case CodecError::UnsupportFormat:
			return "Unsupported format";
		case CodecError::ExternalFailed:
			return "External codec failed";
		case CodecError::FailedToOpenFile:
			return "Failed to open file";
		case CodecError::FailedToCallDecoder:
			return "Failed to call decoder";
		case CodecError::EndOfFile:
			return "End of file";
		case CodecError::Repeat:
			return "Repeat";
		case CodecError::Again:
			return "Try again";
		case CodecError::NotFound:
			return "Not found";
		case CodecError::NotImplement:
			return "Not implemented";
		case CodecError::InvalidArguments:
			return "Invalid arguments";
		case CodecError::CorruptStream:
			return "Corrupt stream";
		case CodecError::Preparing:
			return "Preparing";
		case CodecError::Success:
			return "Success";
		default:
			return "Unknown codec error";
	}
}

static String CodecFailureText(const char *prefix, CodecError error)
{
	return String{ prefix, StringEncoding::ASCII } + String{ CodecErrorName(error), StringEncoding::ASCII };
}

static String EncoderLastError(Codec *encoder)
{
	if (auto ffCodec = InterpretAs<Vision::FFCodec>(encoder))
	{
		return ffCodec->LastError();
	}
	return {};
}

static String FormatLastError(Vision::MediaFormat *format)
{
	if (auto ffFormat = InterpretAs<Vision::FFFormat>(format))
	{
		return ffFormat->LastError();
	}
	return {};
}

class VideoOutput : public IObject
{
public:
	VideoOutput(const String &filepath, const CodecInfo *pEncodeInfo, uint32_t numEncodeInfo);

	~VideoOutput();

	void EnqueueVideoFrame(Picture &&picture);

	void EnqueueAudioFrame(Picture &&picture);

	CodecError Send(const Picture &picture, int stream);

	CodecError Write(const CodedFrame &codedFrame, int stream);

	void RequestFinish();

	void Join();

	void CloseAndJoin();

	void Close();

	void Bind(const VideoEncodeCallbacks &value);

	bool Blocking() const;

	bool operator!() const;

	String Error() const;

public:
	void SetFilterGraph(const std::shared_ptr<FilterGraphComponent> &graph)
	{
		filterGraph = graph;
	}

private:
	struct MuxPacket
	{
		CodedFrame frame;
		MediaType  type   = MediaType::Data;
		int        stream = -1;
	};

	static constexpr size_t kVideoInputQueueLimit = 6;
	// Audio callbacks run on VideoPlayer workers; keep room for slow hardware
	// video encoders to drain so EOF can still be delivered.
	static constexpr size_t kAudioInputQueueLimit = 512;
	static constexpr size_t kVideoMuxPacketQueueLimit = 96;
	static constexpr size_t kAudioMuxPacketQueueLimit = 2048;

	Codec *GetEncoder(MediaType type) const;

	std::deque<Picture> &GetInputQueue(MediaType type);

	const std::deque<Picture> &GetInputQueue(MediaType type) const;

	std::deque<MuxPacket> &GetMuxQueue(MediaType type);

	const std::deque<MuxPacket> &GetMuxQueue(MediaType type) const;

	size_t GetInputQueueLimit(MediaType type) const;

	size_t GetMuxQueueLimit(MediaType type) const;

	Rational GetMuxTimebase(MediaType type) const;

	bool IsInputClosed(MediaType type) const;

	void SetInputClosed(MediaType type);

	bool IsEncodeFinished(MediaType type) const;

	void SetEncodeFinished(MediaType type);

	bool IsEncoderFlushed(MediaType type) const;

	void SetEncoderFlushed(MediaType type);

	bool AllEncodeFinished() const;

	void UpdateQueueCounters();

	void LogStateLocked(const char *where) const;

	bool CancelRequested();

	void PushInputFrame(MediaType type, Picture &&picture);

	bool PopInputFrame(MediaType type, Picture &picture);

	void CloseInput(MediaType type);

	void EncodeLoop(MediaType type);

	void EncodePicture(MediaType type, Picture &&picture);

	bool DrainEncoder(MediaType type);

	void FlushEncoder(MediaType type);

	void PushMuxPacket(MediaType type, CodedFrame &&frame);

	bool CanSelectMuxPacketLocked() const;

	MediaType SelectMuxPacketTypeLocked() const;

	void MuxLoop();

	void ReportProgress(const MuxPacket &packet, int frame);

	void SignalFailure(const String &message = String{});

	void SetError(const String &message);

private:
	LightArray<Codec *, 4> codecs;

	Ref<Codec> videoEncoder;

	Ref<Codec> audioEncoder;

	std::vector<MediaType> mediaTypes;

	URef<Vision::MediaFormat> muxer;

	Thread videoEncodeThread;

	Thread audioEncodeThread;

	Thread muxThread;

	std::deque<Picture> videoInputQueue;

	std::deque<Picture> audioInputQueue;

	std::deque<MuxPacket> videoMuxPacketQueue;

	std::deque<MuxPacket> audioMuxPacketQueue;

	mutable std::mutex stateMutex;

	std::condition_variable inputReadyCondition;

	std::condition_variable inputSpaceCondition;

	std::condition_variable muxReadyCondition;

	std::condition_variable muxSpaceCondition;

	std::mutex callbackMutex;

	bool videoInputClosed = false;

	bool audioInputClosed = false;

	bool videoEncodeFinished = true;

	bool audioEncodeFinished = true;

	bool videoEncoderFlushed = false;

	bool audioEncoderFlushed = false;

	std::atomic_bool failed = false;

	String errorMessage;

	int64_t timestamp = 0;

	int64_t audioTimestamp = 0;

	Timer timer;

	std::atomic<int> muxedVideoFrames = 0;

	std::atomic<size_t> queuedVideoFrames = 0;

	std::atomic<size_t> queuedAudioFrames = 0;

	std::atomic<size_t> queuedMuxPackets = 0;

	Rational progressTimebase{};

	VideoEncodeCallbacks callbacks;

	std::shared_ptr<FilterGraphComponent> filterGraph;

	int streamIndex[4] = {};

	Rational muxTimebase[4] = {};
};

VideoOutput::VideoOutput(const String &filepath, const CodecInfo *pEncodeInfo, uint32_t numEncodeInfo) :
    codecs{},
    videoEncoder{},
    audioEncoder{},
    mediaTypes{},
    muxer{},
    videoEncodeThread{},
    audioEncodeThread{},
    muxThread{},
    timer{}
{
	std::fill(std::begin(streamIndex), std::end(streamIndex), -1);
	codecs.resize(numEncodeInfo);
	mediaTypes.resize(numEncodeInfo, MediaType::Data);

	bool image = FileSystem::IsImage(filepath);
	ImageEncodeInfo imageEncodeInfo = {
	    .quality = 100
	};

	CodecInfo videoEncodeInfo = {};
	for (uint32_t i = 0; i < numEncodeInfo; i++)
	{
		const auto &encodeInfo = pEncodeInfo[i];
		mediaTypes[i] = encodeInfo.mediaType;
		switch (encodeInfo.mediaType)
		{
			case MediaType::Video:
				videoEncodeInfo = encodeInfo;
				if (image)
				{
					videoEncoder = Vision::SelectSuitableCodec(filepath, false, imageEncodeInfo);
				}
				else
				{
					Ref<Codec> encoder = new Vision::FFCodec{ encodeInfo };
					auto ffCodec = encoder.InterpretAs<Vision::FFCodec>();
					if (!ffCodec || !*ffCodec)
					{
						const String codecError = ffCodec ? ffCodec->LastError() : String{};
						SetError(codecError.empty() ? String{ "Failed to create video encoder.", StringEncoding::ASCII } : codecError);
						return;
					}
					videoEncoder = encoder;
				}
				codecs[i] = videoEncoder.Get();
				streamIndex[(int)MediaType::Video] = (int)i;
				break;

			case MediaType::Audio:
				if (!image)
				{
					Ref<Codec> encoder = new Vision::FFCodec{ encodeInfo };
					auto ffCodec = encoder.InterpretAs<Vision::FFCodec>();
					if (!ffCodec || !*ffCodec)
					{
						const String codecError = ffCodec ? ffCodec->LastError() : String{};
						SetError(codecError.empty() ? String{ "Failed to create audio encoder.", StringEncoding::ASCII } : codecError);
						return;
					}
					audioEncoder = encoder;
					codecs[i] = audioEncoder.Get();
					streamIndex[(int)MediaType::Audio] = (int)i;
				}
				break;

			default:
				break;
		}
	}

	if (image)
	{
		muxer = new Vision::ImageFormat;
	}
	else
	{
		muxer = new Vision::FFFormat;
	}

	const CodecError muxOpenResult = muxer->Open(filepath, codecs.data(), pEncodeInfo, numEncodeInfo);
	if (muxOpenResult != CodecError::Success)
	{
		const String muxerError = FormatLastError(muxer.Get());
		SetError(muxerError.empty() ? CodecFailureText("Failed to open output muxer: ", muxOpenResult) : muxerError);
		muxer.Reset();
		return;
	}

	if (videoEncoder)
	{
		if (auto ffCodec = videoEncoder.InterpretAs<Vision::FFCodec>())
		{
			muxTimebase[(int)MediaType::Video] = ffCodec->GetTimebase();
		}
	}
	if (audioEncoder)
	{
		if (auto ffCodec = audioEncoder.InterpretAs<Vision::FFCodec>())
		{
			muxTimebase[(int)MediaType::Audio] = ffCodec->GetTimebase();
		}
	}

	videoInputClosed = !videoEncoder;
	audioInputClosed = !audioEncoder;
	videoEncodeFinished = !videoEncoder;
	audioEncodeFinished = !audioEncoder;
	videoEncoderFlushed = !videoEncoder;
	audioEncoderFlushed = !audioEncoder;
	if (videoEncoder)
	{
		progressTimebase = videoEncodeInfo.timeBase * videoEncodeInfo.framerate;
		videoEncodeThread = std::move(Thread{ [this] {
			EncodeLoop(MediaType::Video);
		} });
		videoEncodeThread.SetDescription("VideoOutput::VideoEncode");
	}
	if (audioEncoder)
	{
		audioEncodeThread = std::move(Thread{ [this] {
			EncodeLoop(MediaType::Audio);
		} });
		audioEncodeThread.SetDescription("VideoOutput::AudioEncode");
	}

	muxThread = std::move(Thread{ [this] {
		MuxLoop();
	} });
	muxThread.SetDescription("VideoOutput::Mux");

#ifdef IMMORTAL_HAVE_VIDEO_PLAYER_STATISTIC
	timer.Start();
#endif
}

VideoOutput::~VideoOutput()
{
	CloseAndJoin();
}

Codec *VideoOutput::GetEncoder(MediaType type) const
{
	switch (type)
	{
		case MediaType::Video:
			return videoEncoder.Get();

		case MediaType::Audio:
			return audioEncoder.Get();

		default:
			return nullptr;
	}
}

std::deque<Picture> &VideoOutput::GetInputQueue(MediaType type)
{
	return type == MediaType::Audio ? audioInputQueue : videoInputQueue;
}

const std::deque<Picture> &VideoOutput::GetInputQueue(MediaType type) const
{
	return type == MediaType::Audio ? audioInputQueue : videoInputQueue;
}

std::deque<VideoOutput::MuxPacket> &VideoOutput::GetMuxQueue(MediaType type)
{
	return type == MediaType::Audio ? audioMuxPacketQueue : videoMuxPacketQueue;
}

const std::deque<VideoOutput::MuxPacket> &VideoOutput::GetMuxQueue(MediaType type) const
{
	return type == MediaType::Audio ? audioMuxPacketQueue : videoMuxPacketQueue;
}

size_t VideoOutput::GetInputQueueLimit(MediaType type) const
{
	return type == MediaType::Audio ? kAudioInputQueueLimit : kVideoInputQueueLimit;
}

size_t VideoOutput::GetMuxQueueLimit(MediaType type) const
{
	return type == MediaType::Audio ? kAudioMuxPacketQueueLimit : kVideoMuxPacketQueueLimit;
}

Rational VideoOutput::GetMuxTimebase(MediaType type) const
{
	const int index = (int)type;
	if (index < 0 || index >= 4)
	{
		return {};
	}
	return muxTimebase[index];
}

bool VideoOutput::IsInputClosed(MediaType type) const
{
	return type == MediaType::Audio ? audioInputClosed : videoInputClosed;
}

void VideoOutput::SetInputClosed(MediaType type)
{
	if (type == MediaType::Audio)
	{
		audioInputClosed = true;
	}
	else
	{
		videoInputClosed = true;
	}
}

bool VideoOutput::IsEncodeFinished(MediaType type) const
{
	return type == MediaType::Audio ? audioEncodeFinished : videoEncodeFinished;
}

void VideoOutput::SetEncodeFinished(MediaType type)
{
	if (type == MediaType::Audio)
	{
		audioEncodeFinished = true;
	}
	else
	{
		videoEncodeFinished = true;
	}
}

bool VideoOutput::IsEncoderFlushed(MediaType type) const
{
	return type == MediaType::Audio ? audioEncoderFlushed : videoEncoderFlushed;
}

void VideoOutput::SetEncoderFlushed(MediaType type)
{
	if (type == MediaType::Audio)
	{
		audioEncoderFlushed = true;
	}
	else
	{
		videoEncoderFlushed = true;
	}
}

bool VideoOutput::AllEncodeFinished() const
{
	return (!videoEncoder || videoEncodeFinished) && (!audioEncoder || audioEncodeFinished);
}

void VideoOutput::UpdateQueueCounters()
{
	queuedVideoFrames.store(videoInputQueue.size(), std::memory_order_release);
	queuedAudioFrames.store(audioInputQueue.size(), std::memory_order_release);
	queuedMuxPackets.store(videoMuxPacketQueue.size() + audioMuxPacketQueue.size(), std::memory_order_release);
}

void VideoOutput::LogStateLocked(const char *where) const
{
	LOG::WARN(
		"VideoOutput stalled at {}: videoIn={} audioIn={} videoMux={} audioMux={} "
		"videoClosed={} audioClosed={} videoFinished={} audioFinished={} "
		"videoFlushed={} audioFlushed={} failed={} muxedVideo={}",
		where,
		videoInputQueue.size(),
		audioInputQueue.size(),
		videoMuxPacketQueue.size(),
		audioMuxPacketQueue.size(),
		videoInputClosed,
		audioInputClosed,
		videoEncodeFinished,
		audioEncodeFinished,
		videoEncoderFlushed,
		audioEncoderFlushed,
		failed.load(std::memory_order_acquire),
		muxedVideoFrames.load(std::memory_order_relaxed));
}

bool VideoOutput::CancelRequested()
{
	std::lock_guard lock{ callbackMutex };
	return callbacks.progressListener && callbacks.progressListener->IsCanceled();
}

void VideoOutput::PushInputFrame(MediaType type, Picture &&picture)
{
	if (!muxer || !GetEncoder(type))
	{
		return;
	}
	if (CancelRequested())
	{
		RequestFinish();
		return;
	}

	if (picture.GetFlags() & Vision::PictureFlags::Eof)
	{
		CloseInput(type);
		return;
	}

	std::unique_lock lock{ stateMutex };
	std::deque<Picture> &queue = GetInputQueue(type);
	const size_t limit = GetInputQueueLimit(type);
	while (!inputSpaceCondition.wait_for(lock, std::chrono::seconds{ 5 }, [&] {
		return failed.load(std::memory_order_acquire) || CancelRequested() || IsInputClosed(type) || queue.size() < limit;
	}))
	{
		LogStateLocked(type == MediaType::Audio ? "audio input space" : "video input space");
	}
	if (CancelRequested())
	{
		videoInputClosed = true;
		audioInputClosed = true;
		lock.unlock();
		inputReadyCondition.notify_all();
		inputSpaceCondition.notify_all();
		muxReadyCondition.notify_all();
		muxSpaceCondition.notify_all();
		return;
	}
	if (failed.load(std::memory_order_acquire) || IsInputClosed(type))
	{
		return;
	}

	queue.emplace_back(std::move(picture));
	UpdateQueueCounters();
	lock.unlock();
	inputReadyCondition.notify_all();
}

bool VideoOutput::PopInputFrame(MediaType type, Picture &picture)
{
	std::unique_lock lock{ stateMutex };
	std::deque<Picture> &queue = GetInputQueue(type);
	while (!inputReadyCondition.wait_for(lock, std::chrono::seconds{ 5 }, [&] {
		return failed.load(std::memory_order_acquire) || CancelRequested() || !queue.empty() || IsInputClosed(type);
	}))
	{
		LogStateLocked(type == MediaType::Audio ? "audio input ready" : "video input ready");
	}
	if (CancelRequested())
	{
		videoInputClosed = true;
		audioInputClosed = true;
		lock.unlock();
		inputReadyCondition.notify_all();
		inputSpaceCondition.notify_all();
		muxReadyCondition.notify_all();
		return false;
	}
	if (failed.load(std::memory_order_acquire) || queue.empty())
	{
		return false;
	}

	picture = std::move(queue.front());
	queue.pop_front();
	UpdateQueueCounters();
	lock.unlock();
	inputSpaceCondition.notify_all();
	return true;
}

void VideoOutput::CloseInput(MediaType type)
{
	{
		std::lock_guard lock{ stateMutex };
		SetInputClosed(type);
	}
	inputReadyCondition.notify_all();
	inputSpaceCondition.notify_all();
}

void VideoOutput::EncodeLoop(MediaType type)
{
	Picture picture;
	while (PopInputFrame(type, picture))
	{
		EncodePicture(type, std::move(picture));
		picture = {};
		if (failed.load(std::memory_order_acquire))
		{
			break;
		}
	}

	if (!failed.load(std::memory_order_acquire) && !CancelRequested())
	{
		DrainEncoder(type);
		FlushEncoder(type);
		DrainEncoder(type);
	}

	{
		std::lock_guard lock{ stateMutex };
		SetEncodeFinished(type);
	}
	muxReadyCondition.notify_all();
	muxSpaceCondition.notify_all();
}

void VideoOutput::EncodePicture(MediaType type, Picture &&picture)
{
	Codec *encoder = GetEncoder(type);
	if (!encoder)
	{
		return;
	}

	if (type == MediaType::Video)
	{
		const int64_t fallbackTimestamp = timestamp++;
		if (!HasValidPictureTimestamp(picture))
		{
			picture.SetTimestamp(fallbackTimestamp);
		}
	}
	else if (type == MediaType::Audio)
	{
		picture.SetTimestamp(audioTimestamp);
		audioTimestamp += picture.GetWidth();
	}

	CodedFrame codedFrame{};
	CodecError ret = CodecError::Again;
	for (int retry = 0; retry < 8 && ret == CodecError::Again && !failed.load(std::memory_order_acquire) && !CancelRequested(); ++retry)
	{
		ret = encoder->Encode(picture, codedFrame);
		if (ret == CodecError::Again)
		{
			DrainEncoder(type);
		}
	}
	if (CancelRequested())
	{
		return;
	}
	if (ret != CodecError::Success)
	{
		LOG::ERR("Failed to encode {} frame!", type == MediaType::Video ? "video" : "audio");
		const String encoderError = EncoderLastError(encoder);
		SignalFailure(encoderError.empty()
			? CodecFailureText(type == MediaType::Video ? "Failed to encode video frame: " : "Failed to encode audio frame: ", ret)
			: encoderError);
		return;
	}

	PushMuxPacket(type, std::move(codedFrame));
	DrainEncoder(type);
}

bool VideoOutput::DrainEncoder(MediaType type)
{
	Codec *encoder = GetEncoder(type);
	if (!encoder)
	{
		return false;
	}

	bool drained = false;
	CodedFrame codedFrame{};
	while (!CancelRequested() && (codedFrame = encoder->GetCodedFrame()))
	{
		drained = true;
		PushMuxPacket(type, std::move(codedFrame));
		codedFrame = {};
		if (failed)
		{
			return drained;
		}
	}
	return drained;
}

void VideoOutput::FlushEncoder(MediaType type)
{
	Codec *encoder = GetEncoder(type);
	if (!encoder || IsEncoderFlushed(type))
	{
		return;
	}

	encoder->Flush();
	SetEncoderFlushed(type);
}

void VideoOutput::PushMuxPacket(MediaType type, CodedFrame &&frame)
{
	if (!frame)
	{
		return;
	}

	const int stream = streamIndex[(int)type];
	if (stream < 0)
	{
		return;
	}
	frame.SetType(type);

	std::unique_lock lock{ stateMutex };
	std::deque<MuxPacket> &queue = GetMuxQueue(type);
	const size_t limit = GetMuxQueueLimit(type);
	while (!muxSpaceCondition.wait_for(lock, std::chrono::seconds{ 5 }, [&] {
		return failed.load(std::memory_order_acquire) || CancelRequested() || queue.size() < limit;
	}))
	{
		LogStateLocked(type == MediaType::Audio ? "audio mux space" : "video mux space");
	}
	if (failed.load(std::memory_order_acquire) || CancelRequested())
	{
		return;
	}

	queue.emplace_back(MuxPacket{ std::move(frame), type, stream });
	UpdateQueueCounters();
	lock.unlock();
	muxReadyCondition.notify_one();
}

bool VideoOutput::CanSelectMuxPacketLocked() const
{
	return SelectMuxPacketTypeLocked() != MediaType::Data;
}

MediaType VideoOutput::SelectMuxPacketTypeLocked() const
{
	const bool hasVideo = !videoMuxPacketQueue.empty();
	const bool hasAudio = !audioMuxPacketQueue.empty();
	if (hasVideo && hasAudio)
	{
		const MuxPacket &videoPacket = videoMuxPacketQueue.front();
		const MuxPacket &audioPacket = audioMuxPacketQueue.front();
		return CompareTimestamp(
			videoPacket.frame.GetTimestamp(),
			GetMuxTimebase(MediaType::Video),
			audioPacket.frame.GetTimestamp(),
			GetMuxTimebase(MediaType::Audio)) <= 0 ? MediaType::Video : MediaType::Audio;
	}
	if (hasVideo)
	{
		return MediaType::Video;
	}
	if (hasAudio)
	{
		return MediaType::Audio;
	}
	return MediaType::Data;
}

void VideoOutput::MuxLoop()
{
	while (true)
	{
		MuxPacket packet{};
		{
			std::unique_lock lock{ stateMutex };
			while (!muxReadyCondition.wait_for(lock, std::chrono::seconds{ 5 }, [&] {
				return failed.load(std::memory_order_acquire) || CancelRequested() || CanSelectMuxPacketLocked() || AllEncodeFinished();
			}))
			{
				LogStateLocked("mux ready");
			}
			if (failed.load(std::memory_order_acquire) || CancelRequested())
			{
				break;
			}

			const MediaType selectedType = SelectMuxPacketTypeLocked();
			if (selectedType == MediaType::Data)
			{
				if (AllEncodeFinished())
				{
					break;
				}
				continue;
			}

			std::deque<MuxPacket> &queue = GetMuxQueue(selectedType);
			packet = std::move(queue.front());
			queue.pop_front();
			UpdateQueueCounters();
		}
		muxSpaceCondition.notify_all();

		const CodecError muxResult = muxer->Write(packet.frame, packet.stream);
		if (muxResult != CodecError::Success)
		{
			const String muxerError = FormatLastError(muxer.Get());
			SignalFailure(muxerError.empty() ? CodecFailureText("Failed to write output packet: ", muxResult) : muxerError);
			break;
		}

		if (packet.type == MediaType::Video)
		{
			const int frame = muxedVideoFrames.fetch_add(1, std::memory_order_relaxed) + 1;
			ReportProgress(packet, frame);
		}
	}

	if (muxer)
	{
		LOG::INFO("VideoOutput closing muxer: muxedVideo={}", muxedVideoFrames.load(std::memory_order_relaxed));
		muxer->Close();
		const String muxerError = FormatLastError(muxer.Get());
		if (!muxerError.empty())
		{
			SignalFailure(muxerError);
		}
		LOG::INFO("VideoOutput closed muxer: muxedVideo={}", muxedVideoFrames.load(std::memory_order_relaxed));
	}

#ifdef IMMORTAL_HAVE_VIDEO_PLAYER_STATISTIC
	const double time = timer.Duration();
	const int frames = muxedVideoFrames.load(std::memory_order_relaxed);
	LOG::INFO("Encoding Statistic: frames:{}, time:{}, fps:{}", frames, time, time > 0.0 ? frames / time : 0.0);
#endif
}

void VideoOutput::ReportProgress(const MuxPacket &packet, int frame)
{
	VideoEncodeCallbacks localCallbacks;
	{
		std::lock_guard lock{ callbackMutex };
		localCallbacks = callbacks;
	}

	if (localCallbacks.ReportProgress)
	{
		localCallbacks.ReportProgress(frame, packet.frame.GetTimestamp(), progressTimebase);
	}
}

void VideoOutput::SignalFailure(const String &message)
{
	{
		std::lock_guard lock{ stateMutex };
		if (!message.empty() && errorMessage.empty())
		{
			errorMessage = message;
		}
		failed.store(true, std::memory_order_release);
		videoInputClosed = true;
		audioInputClosed = true;
	}
	inputReadyCondition.notify_all();
	inputSpaceCondition.notify_all();
	muxReadyCondition.notify_all();
	muxSpaceCondition.notify_all();
}

void VideoOutput::SetError(const String &message)
{
	if (message.empty())
	{
		return;
	}

	std::lock_guard lock{ stateMutex };
	if (errorMessage.empty())
	{
		errorMessage = message;
	}
}

String VideoOutput::Error() const
{
	std::lock_guard lock{ stateMutex };
	return errorMessage;
}

void VideoOutput::EnqueueVideoFrame(Picture &&picture)
{
	PushInputFrame(MediaType::Video, std::move(picture));
}

void VideoOutput::EnqueueAudioFrame(Picture &&picture)
{
	PushInputFrame(MediaType::Audio, std::move(picture));
}

CodecError VideoOutput::Send(const Picture &picture, int stream)
{
	if (stream < 0 || stream >= (int)mediaTypes.size())
	{
		return CodecError::InvalidArguments;
	}

	Picture copy = picture;
	switch (mediaTypes[stream])
	{
		case MediaType::Video:
			EnqueueVideoFrame(std::move(copy));
			break;

		case MediaType::Audio:
			EnqueueAudioFrame(std::move(copy));
			break;

		default:
			return CodecError::InvalidArguments;
	}

	return failed.load(std::memory_order_acquire) ? CodecError::ExternalFailed : CodecError::Success;
}

CodecError VideoOutput::Write(const CodedFrame &codedFrame, int stream)
{
	if (!muxer || stream < 0)
	{
		return CodecError::InvalidArguments;
	}
	return muxer->Write(codedFrame, stream);
}

void VideoOutput::RequestFinish()
{
	{
		std::lock_guard lock{ stateMutex };
		videoInputClosed = true;
		audioInputClosed = true;
	}
	inputReadyCondition.notify_all();
	inputSpaceCondition.notify_all();
	muxReadyCondition.notify_all();
	muxSpaceCondition.notify_all();
}

void VideoOutput::Join()
{
	videoEncodeThread.Join();
	audioEncodeThread.Join();
	muxReadyCondition.notify_all();
	muxThread.Join();
}

void VideoOutput::CloseAndJoin()
{
	RequestFinish();
	Join();
}

void VideoOutput::Close()
{
	CloseAndJoin();
}

void VideoOutput::Bind(const VideoEncodeCallbacks &value)
{
	std::lock_guard lock{ callbackMutex };
	callbacks = value;
}

bool VideoOutput::Blocking() const
{
	return queuedVideoFrames.load(std::memory_order_acquire) >= kVideoInputQueueLimit ||
	       queuedAudioFrames.load(std::memory_order_acquire) >= kAudioInputQueueLimit ||
	       queuedMuxPackets.load(std::memory_order_acquire) >= kVideoMuxPacketQueueLimit + kAudioMuxPacketQueueLimit;
}

bool VideoOutput::operator!() const
{
	return !muxer;
}

VideoOutputComponent::VideoOutputComponent() :
    v{}
{

}

VideoOutputComponent::VideoOutputComponent(const String &filepath, const CodecInfo *pEncodeInfo, uint32_t numEncodeInfo) :
    v{ new VideoOutput{ filepath, pEncodeInfo, numEncodeInfo } }
{

}

VideoOutputComponent::~VideoOutputComponent()
{
	v.Reset();
}

void VideoOutputComponent::EnqueueVideoFrame(Picture &&picture)
{
	if (!v || !(*v))
	{
		return;
	}
	v->EnqueueVideoFrame(std::move(picture));
}

void VideoOutputComponent::EnqueueAudioFrame(Picture &&picture)
{
	if (!v || !(*v))
	{
		return;
	}
	v->EnqueueAudioFrame(std::move(picture));
}

CodecError VideoOutputComponent::Write(const CodedFrame &codedFrame, int stream)
{
	if (!v || !(*v))
	{
		return CodecError::InvalidArguments;
	}
	return v->Write(codedFrame, stream);
}

void VideoOutputComponent::SetFilterGraph(const std::shared_ptr<FilterGraphComponent> &graph)
{
	if (!v || !(*v))
	{
		return;
	}
	v->SetFilterGraph(graph);
}

void VideoOutputComponent::RequestFinish()
{
	if (!v)
	{
		return;
	}
	v->RequestFinish();
}

void VideoOutputComponent::Join()
{
	if (!v)
	{
		return;
	}
	v->Join();
}

void VideoOutputComponent::CloseAndJoin()
{
	if (!v)
	{
		return;
	}
	v->CloseAndJoin();
}

void VideoOutputComponent::Close()
{
	CloseAndJoin();
}

void VideoOutputComponent::Bind(const VideoEncodeCallbacks &value)
{
	if (!v || !(*v))
	{
		return;
	}
	v->Bind(value);
}

String VideoOutputComponent::Error() const
{
	return v ? v->Error() : String{};
}

bool VideoOutputComponent::Blocking() const
{
	if (!v || !(*v))
	{
		return false;
	}
	return v->Blocking();
}

VideoOutputComponent::operator bool() const
{
	if (!v)
	{
		return false;
	}
	return !!(*v);
}

void VideoOutputComponent::Swap(VideoOutputComponent &other)
{
	v.Swap(other.v);
}

}
