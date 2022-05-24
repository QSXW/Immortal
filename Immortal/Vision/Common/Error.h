#pragma once

#ifndef IMMORTAL_ERROR_H__
#define IMMORTAL_ERROR_H__

namespace Immortal
{

#define NEG(N) -(N)

enum CodecError
{
    CorruptedBitstream = NEG(1114),
    UnsupportFormat,
    ExternalFailed,
    FailedToOpenFile,
    FailedToCallDecoder,
    EndOfFile,
    Repeat,
    Succeed = 0,
    Preparing
};

}

#endif
