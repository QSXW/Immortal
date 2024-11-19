#pragma once

#include "Scene/Component.h"
#include "Video/FFCodec.h"
#include "Demux/FFDemuxer.h"
#include "Shared/Async.h"
#include "Framework/Timer.h"

namespace Immortal
{

struct VideoEncodeCallbacks
{
    std::function<void(int frame, int64_t timestamp, Rational timebase)> ReportProgress;
};

class VideoOutputComponent
{
public:
    VideoOutputComponent(const String &filepath, const EncodeInfo &videoEncodeInfo, const EncodeInfo &audioEncodeInfo, const std::initializer_list<MediaType> &&streamInfos);

    CodecError Send(const Picture &picture, int stream);

    CodecError Write(const CodedFrame &codedFrame, int stream);

    void Close();

    void Join()
    {
        muxThread.Join();
    }

    void Bind(const VideoEncodeCallbacks &value)
    {
        callbacks = value;
    }

    bool Blocking() const
    {
        return videoEncodeThread.TaskSize() > 3;
    }

    void EnqueueVideoFrame(Picture &&picture);

    void EnqueueAudioFrame(Picture &&picture);

protected:
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
};

}
