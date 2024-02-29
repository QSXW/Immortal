#include <iostream>

#include "Immortal.h"
#include "FileSystem/Stream.h"
#include "Vision/Image.h"
#include "Vision/Video/Video.h"
#include "Vision/CodedFrame.h"
#include "Vision/Video/VVC.h"

using namespace Immortal;

int main()
{
    LOG::Setup();

    {
        std::string path = "C:\\SDK\\C\\tests\\conformance\\passed\\v1/8b420_B_Bytedance_2.bit";

		Vision::VVCCodec decoder;

        Vision::FFDemuxer demuxer;
		demuxer.Open(path, &decoder);

        while (true)
        {
			Vision::CodedFrame codedFrame;
            if (demuxer.Read(&codedFrame) == CodecError::Succeed)
            {
				decoder.Decode(codedFrame);
            }
        }
    }

    return 0;
}
