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

class VideoOutput;
class VideoOutputComponent
{
public:
    VideoOutputComponent(const String &filepath, const EncodeInfo &videoEncodeInfo, const EncodeInfo &audioEncodeInfo, const std::vector<MediaType> &streamInfos);

    void EnqueueVideoFrame(Picture &&picture);

	void EnqueueAudioFrame(Picture &&picture);

    CodecError Send(const Picture &picture, int stream);

    CodecError Write(const CodedFrame &codedFrame, int stream);

    void Join();

    void Close();

    void Bind(const VideoEncodeCallbacks &value);

    bool Blocking() const;

    operator bool() const;

protected:
	URef<VideoOutput> v;
};

}
