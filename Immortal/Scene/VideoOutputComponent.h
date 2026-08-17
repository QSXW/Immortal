#pragma once

#include "Scene/Component.h"
#include "Video/FFCodec.h"
#include "MediaFormat/FFFormat.h"
#include "Shared/Async.h"
#include "Framework/Timer.h"
#include "Vision/Types.h"

namespace Immortal
{

struct VideoEncodeCallbacks
{
    using ProgressListenerType = ProgressListener;

    std::function<void(int frame, int64_t timestamp, Rational timebase)> ReportProgress;

    Ref<ProgressListenerType> progressListener;
};

class VideoOutput;
class VideoOutputComponent : public IObject, public Component
{
public:
	SL_SWAPPABLE(VideoOutputComponent)

public:
	VideoOutputComponent();

    VideoOutputComponent(const String &filepath, const CodecInfo *pEncodeInfo, uint32_t numEncodeInfo);

    ~VideoOutputComponent();

    void EnqueueVideoFrame(Picture &&picture);

	void EnqueueAudioFrame(Picture &&picture);

    CodecError Send(const Picture &picture, int stream);

    CodecError Write(const CodedFrame &codedFrame, int stream);

    void SetFilterGraph(const std::shared_ptr<FilterGraphComponent> &graph);

    // Stop accepting new frames for active streams without waiting. Use when the
    // producer wants to finish feeding frames but cannot block yet.
    void RequestFinish();

    // Wait for already-finished streams to drain. The producer must have sent
    // EOF frames or called RequestFinish()/CloseAndJoin().
    void Join();

    // Stop accepting frames, drain encoder/mux workers, close the muxer, and wait.
    void CloseAndJoin();

    // Legacy alias for CloseAndJoin().
    void Close();

    void Bind(const VideoEncodeCallbacks &value);

    String Error() const;

    bool Blocking() const;

    operator bool() const;

	void Swap(VideoOutputComponent &other);

protected:
	URef<VideoOutput> v;
};

}
