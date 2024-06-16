#pragma once

#ifndef IMMORTAL_ERROR_H__
#define IMMORTAL_ERROR_H__

namespace Immortal
{

#define NEG(N) -(N)

enum class CodecError
{
	CorruptedBitstream = NEG(1114),
	OutOfMemory,
	UnsupportFormat,
	ExternalFailed,
	FailedToOpenFile,
	FailedToCallDecoder,
	EndOfFile,
	Repeat,
	Again,
	NotImplement,
	InvalidArguments,
    CorruptStream,
	Success = 0,
    Preparing
};

}

#endif
