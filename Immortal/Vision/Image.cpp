#include "Image.h"
#include "Codec.h"
#include "Image/ImageCodec.h"
#include "FileSystem/FileSystem.h"
#include "Vision/MediaFormat/FFFormat.h"
#include "Vision/Video/FFCodec.h"

namespace Immortal
{
namespace Vision
{

static CodedFrame WriteDemuxedStillImage(const Picture &picture, const String &path, const ImageEncodeInfo &encodeInfo)
{
#if HAVE_FFMPEG
    (void)encodeInfo;
    if (path.empty())
    {
        return {};
    }

    CodecInfo info{
        .mediaType = MediaType::Video,
        .codecId = CodecId::AVIF,
        .width = picture.GetWidth(),
        .height = picture.GetHeight(),
        .format = picture.GetFormat(),
        .bitRate = 0,
        .gopSize = 1,
        .framerate = { 1, 1 },
        .timeBase = { 1, 1 },
        .sampleAspectRatio = picture.GetSampleAspectRatio(),
        .displayOrientation = {}
    };
    if (const DisplayOrientation *orientation = picture.GetProperty<DisplayOrientation>())
    {
        info.displayOrientation = *orientation;
    }

    URef<FFCodec> encoder = new FFCodec{ info };
    if (!encoder || !encoder->GetHandle())
    {
        return {};
    }

    URef<FFFormat> muxer = new FFFormat;
    Codec *codec = encoder.Get();
    if (muxer->Open(path, &codec, &info, 1) != CodecError::Success)
    {
        return {};
    }

    CodedFrame codedFrame;
    if (encoder->Encode(picture, codedFrame) != CodecError::Success)
    {
        return {};
    }
    encoder->Flush();

    codedFrame = encoder->GetCodedFrame();
    if (!codedFrame || muxer->Write(codedFrame) != CodecError::Success)
    {
        return {};
    }
    muxer->Close();

    return CodedFrame{ FileSystem::ReadBinary(path) };
#else
    (void)picture;
    (void)path;
    (void)encodeInfo;
    return {};
#endif
}

Codec *SelectSuitableCodec(const std::string &path, bool decoder, const ImageEncodeInfo &info)
{
    (void)decoder;
    const FileFormat fileFormat = FileSystem::DumpFileId(path);
    if (fileFormat == FileFormat::GPR)
    {
        return new GprCodec{
            Format::RGBA8,
            String{ path, StringEncoding::UTF8 } };
    }

    if (FileSystem::IsRawImage(fileFormat))
    {
        return new RawCodec{ Format::RGBA8 };
    }

    switch (fileFormat)
    {
    case FileFormat::BMP:
        return new BMPCodec;

    case FileFormat::JPG:
    case FileFormat::JPEG:
    case FileFormat::JFIF:
#if HAVE_TURBOJPEG
		return new TurboJpegCodec{info};
#endif
    case FileFormat::MJPG:
#if HAVE_TURBOJPEG
		return new TurboJpegCodec{info};
#endif
		return new FFMjpegCodec;

    case FileFormat::PNG:
#if HAVE_PNG
		return new PNGCodec{info};
#endif
    case FileFormat::HDR:
        return new STBCodec;

    case FileFormat::JXL:
#if HAVE_JXL
		return new JxlCodec;
#endif
		return new FFJpegxlCodec;

    case FileFormat::AVIF:
    case FileFormat::HEIF:
    case FileFormat::HEIC:
    case FileFormat::HIF:
        return new FFVideoImageCodec;

    case FileFormat::TIFF:
    case FileFormat::TIF:
		return new FFTiffCodec;

    case FileFormat::PPM:
    case FileFormat::PGM:
    case FileFormat::PNM:
        return new PPMCodec;

    case FileFormat::WEBP:
#if HAVE_WEBP
		return new WebpCodec{};
#endif
		return new FFWebpCodec;

    case FileFormat::GIF:
    case FileFormat::PSD:
        return new STBCodec;

    case FileFormat::EXR:
        return new FFExrCodec;

    case FileFormat::DPX:
        return new FFDPXCodec;

    case FileFormat::J2K:
        return new FFJpeg2000Codec;

    case FileFormat::TGA:
        return new FFTargaCodec;

    case FileFormat::PIC:
    case FileFormat::PCT:
        return new FFQdrawCodec;

    case FileFormat::SUN:
    case FileFormat::RAS:
        return new FFSunRasterCodec;

    case FileFormat::SGI:
        return new FFSgiCodec;

    case FileFormat::FIT:
    case FileFormat::FITS:
        return new FFFitsCodec;

    case FileFormat::ILBM:
        return new FFIffIlbmCodec;

    case FileFormat::XBM:
        return new FFXbmCodec;

    case FileFormat::XFACE:
        return new FFXfaceCodec;

    case FileFormat::JLS:
        return new FFJpegLsCodec;

    default:
        return new OpenCVCodec;
        break;
    }
}

Picture Read(const String &path)
{
    Vision::CodedFrame codedFrame{ FileSystem::ReadBinary(path) };
    if (!codedFrame.GetData())
    {
		return {};
    }

    URef<Interface::Codec> codec = SelectSuitableCodec(path, true);
    if (!codec)
    {
		return {};
    }

    if (codec->Decode(codedFrame) != CodecError::Success)
    {
		return {};
    }

    return codec->GetPicture();
}

CodedFrame Write(const Picture &picture, const String &path, const ImageEncodeInfo &info)
{
    if (FileSystem::IsFormat<FileFormat::AVIF>(path))
    {
        return WriteDemuxedStillImage(picture, path, info);
    }

	URef<Interface::Codec> codec = SelectSuitableCodec(path, false, info);

	CodedFrame codedFrame;
	if (codec->Encode(picture, codedFrame) != CodecError::Success)
	{
		return {};
	}

    if (!path.empty())
    {
		Stream stream{path, StreamMode::Write};
        if (!stream.Writable())
        {
			LOG::ERR("Failed to open file {} to write the encoded image data!", path.c_str());
			return {};
        }
		stream.Write(codedFrame.GetData(), codedFrame.GetSize());
    }

	return codedFrame;
}

}
}
