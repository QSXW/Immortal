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

    Ref<ProgressListener> progressListener;
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

    void Join();

    void Close();

    void Bind(const VideoEncodeCallbacks &value);

    bool Blocking() const;

    operator bool() const;

	void Swap(VideoOutputComponent &other);

protected:
	URef<VideoOutput> v;
};

}
