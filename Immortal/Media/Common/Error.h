#pragma once

#ifndef IMMORTAL_ERROR_H__
#define IMMORTAL_ERROR_H__

namespace Immortal
{

#define NEG(N) -(N)

enum CodecError
{
    CorruptedBitstream = NEG(1114),
    FailedToOpenFile,
    FailedToCallDecoder,
    Succeed = 0,
    Preparing
};

}

#endif
