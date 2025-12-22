#pragma once

#include "Shared/IObject.h"
#include "Codec.h"
#include "Vision/Types.h"
#include "Vision/Common/Error.h"
#include "Vision/Common/Animator.h"
#include "String/IString.h"

namespace Immortal
{

namespace Vision
{

class IMMORTAL_API Demuxer : public IObject
{
public:
	virtual ~Demuxer() = default;

	/**
	 * @brief Open a file
	 */
	virtual CodecError Open(const String &filepath)
	{
		return CodecError::FailedToCallDecoder;
	}

	virtual CodecError Open(const String &filepath, Codec **pCodec, const CodecInfo *encodeInfos, uint32_t numCodec)
	{
	return CodecError::FailedToCallDecoder;
	}

	/**
	 * @brief Close the demuxer,
	 *
	 */
	virtual void Close()
	{

	}

	/**
	 * @brief Read a coded frame or data block
	 */
	virtual CodecError Read(CodedFrame *codedFrame)
	{
		return CodecError::FailedToCallDecoder;
	}

	virtual CodecError Write(const CodedFrame &codedFrame, int stream = 0)
	{
	return CodecError::FailedToCallDecoder;
	}

	virtual CodecError Seek(MediaType type, int64_t pts, int64_t min, int64_t max)
	{
	return CodecError::FailedToCallDecoder;
	}

	virtual CodecError GetStreamInfo(MediaType type, CodecInfo &streamInfo)
	{
	return CodecError::FailedToCallDecoder;
	}

	virtual Animator &GetAnimator(MediaType mediaType)
	{
	static Animator empty;
	return empty;
	}

	virtual const String &GetSource() const = 0;
};

}

using Demuxer = Vision::Demuxer;

}
