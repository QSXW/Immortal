#pragma once

#include <filesystem>

#include "Core.h"
#include "Stream.h"
#include "String/IString.h"
#include "Shared/Log.h"

namespace Immortal
{

#ifdef CreateDirectory
#undef CreateDirectory
#endif

constexpr uint64_t MakeIdentifier(
    uint8_t u0 = 0,
    uint8_t u1 = 0,
    uint8_t u2 = 0,
    uint8_t u3 = 0,
    uint8_t u4 = 0,
    uint8_t u5 = 0,
    uint8_t u6 = 0,
    uint8_t u7 = 0)
{
    return (((uint64_t)u7) << 8 * 7) |
           (((uint64_t)u6) << 8 * 6) |
           (((uint64_t)u5) << 8 * 5) |
           (((uint64_t)u4) << 8 * 4) |
           (((uint64_t)u3) << 8 * 3) |
           (((uint64_t)u2) << 8 * 2) |
           (((uint64_t)u1) << 8 * 1) |
           (((uint64_t)u0) << 8 * 0);
}

enum class FileType
{
    Directory   = BIT(0),
    RegularFile = BIT(1),
    Volumn,
    Desktop,
    Picture,
    Video,
    Audio,
    CPP,
    OBJ,
    EXE,
    MP4,
    MOV,
    JSON,
    PDF,
    HTML,
    AVI,
    BIN,
    BMP,
    DLL,
    DAT,
    DOC,
    GIF,
    JPG,
    JS,
    PNG,
    PPT,
    PSD,
    RAW,
    SQL,
    TIF,
    TXT,
    XML,
    ZIP,
    WEBP,
    JXL,
    TIFF,
    Num
};
SL_ENABLE_BITWISE_OPERATOR(FileType)

enum class FileFlagBits
{
    Empty = BIT(0),
};
SL_ENABLE_BITWISE_OPERATOR(FileFlagBits)

enum class FileFormat : uint64_t
{
    /** 3D Model formats supported by Assimp */
    BLEND = MakeIdentifier('B', 'L', 'E', 'N', 'D'),
    GLTF  = MakeIdentifier('G', 'L', 'T', 'F'     ),
    FBX   = MakeIdentifier('F', 'B', 'X'          ),
    OBJ   = MakeIdentifier('O', 'B', 'J'          ),

    /** Audio formats */
    AAC    = MakeIdentifier('A', 'A', 'C'          ),
    WAV    = MakeIdentifier('W', 'A', 'V'          ),
    FLAC   = MakeIdentifier('F', 'L', 'A', 'C'     ),
    MP3    = MakeIdentifier('M', 'P', '3'          ),
    OPUS   = MakeIdentifier('O', 'P', 'U', 'S'     ),
    AC3    = MakeIdentifier('A', 'C', '3'          ),
    ADTS   = MakeIdentifier('A', 'D', 'T', 'S'     ),
    AEA    = MakeIdentifier('A', 'E', 'A'          ),
    AMR    = MakeIdentifier('A', 'M', 'R'          ),
    APE    = MakeIdentifier('A', 'P', 'E'          ),
    APM    = MakeIdentifier('A', 'P', 'M'          ),
    AWB    = MakeIdentifier('A', 'W', 'B'          ),
    CAF    = MakeIdentifier('C', 'A', 'F'          ),
    DFF    = MakeIdentifier('D', 'F', 'F'          ),
    DTS    = MakeIdentifier('D', 'T', 'S'          ),
    DTSHD  = MakeIdentifier('D', 'T', 'S', 'H', 'D'),
    EAC3   = MakeIdentifier('E', 'A', 'C', '3'     ),
    F32    = MakeIdentifier('F', '3', '2'          ),
    G722   = MakeIdentifier('G', '7', '2', '2'     ),
    IAMF   = MakeIdentifier('I', 'A', 'M', 'F'     ),
    M4A    = MakeIdentifier('M', '4', 'A'          ),
    MLP    = MakeIdentifier('M', 'L', 'P'          ),
    MPC    = MakeIdentifier('M', 'P', 'C'          ),
    NIST   = MakeIdentifier('N', 'I', 'S', 'T'     ),
    OMA    = MakeIdentifier('O', 'M', 'A'          ),
    PCM    = MakeIdentifier('P', 'C', 'M'          ),
    QCP    = MakeIdentifier('Q', 'C', 'P'          ),
    QOA    = MakeIdentifier('Q', 'O', 'A'          ),
    RCV    = MakeIdentifier('R', 'C', 'V'          ),
    SHN    = MakeIdentifier('S', 'H', 'N'          ),
    TAK    = MakeIdentifier('T', 'A', 'K'          ),
    TCO    = MakeIdentifier('T', 'C', 'O'          ),
    THD    = MakeIdentifier('T', 'H', 'D'          ),
    TTA    = MakeIdentifier('T', 'T', 'A'          ),
    TUN    = MakeIdentifier('T', 'U', 'N'          ),
    VOC    = MakeIdentifier('V', 'O', 'C'          ),
    VQF    = MakeIdentifier('V', 'Q', 'F'          ),
    W64    = MakeIdentifier('W', '6', '4'          ),
    WMA    = MakeIdentifier('W', 'M', 'A'          ),
    WV     = MakeIdentifier('W', 'V'               ),
    WVE    = MakeIdentifier('W', 'V', 'E'          ),
    XWMA   = MakeIdentifier('X', 'W', 'M', 'A'     ),

    /** Still Image formats */
    BMP   = MakeIdentifier('B', 'M', 'P'     ),
    PNG   = MakeIdentifier('P', 'N', 'G'     ),
    PPM   = MakeIdentifier('P', 'P', 'M'     ),
    JFIF  = MakeIdentifier('J', 'F', 'I', 'F'),
    JPG   = MakeIdentifier('J', 'P', 'G'     ),
    JPEG  = MakeIdentifier('J', 'P', 'E', 'G'),
    JXL   = MakeIdentifier('J', 'X', 'L'     ),
    HDR   = MakeIdentifier('H', 'D', 'R'     ),
    ARW   = MakeIdentifier('A', 'R', 'W'     ),
    NEF   = MakeIdentifier('N', 'E', 'F'     ),
    CR2   = MakeIdentifier('C', 'R', '2'     ),
    CR3   = MakeIdentifier('C', 'R', '3'     ),
    AVIF  = MakeIdentifier('A', 'V', 'I', 'F'),
    FFF   = MakeIdentifier('F', 'F', 'F'     ),
    _3FR  = MakeIdentifier('3', 'F', 'R'     ),
    RAF   = MakeIdentifier('R', 'A', 'F'     ),
    EXR   = MakeIdentifier('E', 'X', 'R'     ),
    RW2   = MakeIdentifier('R', 'W', '2'     ),
    WEBP  = MakeIdentifier('W', 'E', 'B', 'P'),
    HEIC  = MakeIdentifier('H', 'E', 'I', 'C'),
    TIFF  = MakeIdentifier('T', 'I', 'F', 'F'),
    DNG   = MakeIdentifier('D', 'N', 'G'     ),

    /** Video file format extensions */
    AVI   = MakeIdentifier('A', 'V', 'I'     ),
    OBU   = MakeIdentifier('O', 'B', 'U'     ),
    IVF   = MakeIdentifier('I', 'V', 'F'     ),
    MP4   = MakeIdentifier('M', 'P', '4'     ),
    VVC   = MakeIdentifier('V', 'V', 'C'     ),
    H264  = MakeIdentifier('H', '2', '6', '4'),
    H265  = MakeIdentifier('H', '2', '6', '5'),
    H266  = MakeIdentifier('H', '2', '6', '6'),
    _266  = MakeIdentifier('2', '6', '6'     ),
    MKV   = MakeIdentifier('M', 'K', 'V'     ),
    TS    = MakeIdentifier('T', 'S'          ),
    MOV   = MakeIdentifier('M', 'O', 'V'     ),
    M2TS  = MakeIdentifier('M', '2', 'T', 'S'),
    WEBM  = MakeIdentifier('W', 'E', 'B', 'M'),
    FLV   = MakeIdentifier('F', 'L', 'V'     ),
    BIT   = MakeIdentifier('B', 'I', 'T'     ),
	MXF   = MakeIdentifier('M', 'X', 'F'     ),
    OGV   = MakeIdentifier('O', 'G', 'V'     ),
    OGG   = MakeIdentifier('O', 'G', 'G'     ),
    VC1   = MakeIdentifier('V', 'C', '1'     ),
    MNG   = MakeIdentifier('M', 'N', 'G'     ),
    QT    = MakeIdentifier('Q', 'T'          ),
    WMV   = MakeIdentifier('W', 'M', 'V'     ),
    RMVB  = MakeIdentifier('R', 'M', 'V', 'B'),
	ASF   = MakeIdentifier('A', 'S', 'F'     ),
	AMV   = MakeIdentifier('A', 'M', 'V'     ),
	M4V   = MakeIdentifier('M', '4', 'V'     ),
	MPG   = MakeIdentifier('M', 'P', 'G'     ),
    _3GP  = MakeIdentifier('3', 'G', 'P'     ),

    /** 3D Lookup Table */
    CUBE  = MakeIdentifier('C', 'U', 'B', 'E'),

    /** Immortal Scene */
    IML   = MakeIdentifier('I', 'M', 'L'     ),

    CPP   = MakeIdentifier('C', 'P', 'P'     ),
    EXE   = MakeIdentifier('E', 'X', 'E'     ),
    JSON  = MakeIdentifier('J', 'S', 'O', 'N'),
    PDF   = MakeIdentifier('P', 'D', 'F'     ),
    HTML  = MakeIdentifier('H', 'T', 'M', 'L'),
    BIN   = MakeIdentifier('B', 'I', 'N'     ),
    DLL   = MakeIdentifier('D', 'L', 'L'     ),
    DAT   = MakeIdentifier('D', 'A', 'T'     ),
    DOC   = MakeIdentifier('D', 'O', 'C'     ),
    GIF   = MakeIdentifier('G', 'I', 'F'     ),
    JS    = MakeIdentifier('J', 'S'          ),
    PPT   = MakeIdentifier('P', 'P', 'T'     ),
    PSD   = MakeIdentifier('P', 'S', 'D'     ),
    SQL   = MakeIdentifier('S', 'Q', 'L'     ),
    TIF   = MakeIdentifier('T', 'I', 'F'     ),
    TXT   = MakeIdentifier('T', 'X', 'T'     ),
    XML   = MakeIdentifier('X', 'M', 'L'     ),
    ZIP   = MakeIdentifier('Z', 'I', 'P'     ),

    /** FFmpeg fate-suite (sample extensions) */
    _11C   = MakeIdentifier('1', '1', 'C'          ),
    _264   = MakeIdentifier('2', '6', '4'          ),
    _26L   = MakeIdentifier('2', '6', 'L'          ),
    _302   = MakeIdentifier('3', '0', '2'          ),
    _44C   = MakeIdentifier('4', '4', 'C'          ),
    _4XM   = MakeIdentifier('4', 'X', 'M'          ),
    _5C    = MakeIdentifier('5', 'C'               ),
    AA     = MakeIdentifier('A', 'A'               ),
    AA3    = MakeIdentifier('A', 'A', '3'          ),
    ACT    = MakeIdentifier('A', 'C', 'T'          ),
    ADP    = MakeIdentifier('A', 'D', 'P'          ),
    ANM    = MakeIdentifier('A', 'N', 'M'          ),
    ANS    = MakeIdentifier('A', 'N', 'S'          ),
    APC    = MakeIdentifier('A', 'P', 'C'          ),
    APV    = MakeIdentifier('A', 'P', 'V'          ),
    AQT    = MakeIdentifier('A', 'Q', 'T'          ),
    ASS    = MakeIdentifier('A', 'S', 'S'          ),
    AST    = MakeIdentifier('A', 'S', 'T'          ),
    AUD    = MakeIdentifier('A', 'U', 'D'          ),
    AVC    = MakeIdentifier('A', 'V', 'C'          ),
    AVS    = MakeIdentifier('A', 'V', 'S'          ),
    BCSTM  = MakeIdentifier('B', 'C', 'S', 'T', 'M'),
    BFI    = MakeIdentifier('B', 'F', 'I'          ),
    BFSTM  = MakeIdentifier('B', 'F', 'S', 'T', 'M'),
    BIK    = MakeIdentifier('B', 'I', 'K'          ),
    BITS   = MakeIdentifier('B', 'I', 'T', 'S'     ),
    BMV    = MakeIdentifier('B', 'M', 'V'          ),
    BRSTM  = MakeIdentifier('B', 'R', 'S', 'T', 'M'),
    BS     = MakeIdentifier('B', 'S'               ),
    C93    = MakeIdentifier('C', '9', '3'          ),
    CAK    = MakeIdentifier('C', 'A', 'K'          ),
    CAM    = MakeIdentifier('C', 'A', 'M'          ),
    CDATA  = MakeIdentifier('C', 'D', 'A', 'T', 'A'),
    CDG    = MakeIdentifier('C', 'D', 'G'          ),
    CDXL   = MakeIdentifier('C', 'D', 'X', 'L'     ),
    CIN    = MakeIdentifier('C', 'I', 'N'          ),
    CINE   = MakeIdentifier('C', 'I', 'N', 'E'     ),
    CMV    = MakeIdentifier('C', 'M', 'V'          ),
    CPK    = MakeIdentifier('C', 'P', 'K'          ),
    DCT    = MakeIdentifier('D', 'C', 'T'          ),
    DDS    = MakeIdentifier('D', 'D', 'S'          ),
    DEC    = MakeIdentifier('D', 'E', 'C'          ),
    DEE    = MakeIdentifier('D', 'E', 'E'          ),
    DFA    = MakeIdentifier('D', 'F', 'A'          ),
    DIVX   = MakeIdentifier('D', 'I', 'V', 'X'     ),
    DNXHR  = MakeIdentifier('D', 'N', 'X', 'H', 'R'),
    DPX    = MakeIdentifier('D', 'P', 'X'          ),
    DRC    = MakeIdentifier('D', 'R', 'C'          ),
    DSS    = MakeIdentifier('D', 'S', 'S'          ),
    DUK    = MakeIdentifier('D', 'U', 'K'          ),
    DXA    = MakeIdentifier('D', 'X', 'A'          ),
    EVC    = MakeIdentifier('E', 'V', 'C'          ),
    FIT    = MakeIdentifier('F', 'I', 'T'          ),
    FITS   = MakeIdentifier('F', 'I', 'T', 'S'     ),
    FLI    = MakeIdentifier('F', 'L', 'I'          ),
    FMV    = MakeIdentifier('F', 'M', 'V'          ),
    GDV    = MakeIdentifier('G', 'D', 'V'          ),
    H263   = MakeIdentifier('H', '2', '6', '3'     ),
    HEVC   = MakeIdentifier('H', 'E', 'V', 'C'     ),
    IDX    = MakeIdentifier('I', 'D', 'X'          ),
    IFF    = MakeIdentifier('I', 'F', 'F'          ),
    ILBM   = MakeIdentifier('I', 'L', 'B', 'M'     ),
    ISM    = MakeIdentifier('I', 'S', 'M'          ),
    ISS    = MakeIdentifier('I', 'S', 'S'          ),
    J2K    = MakeIdentifier('J', '2', 'K'          ),
    JLS    = MakeIdentifier('J', 'L', 'S'          ),
    JSS    = MakeIdentifier('J', 'S', 'S'          ),
    JSV    = MakeIdentifier('J', 'S', 'V'          ),
    JV     = MakeIdentifier('J', 'V'               ),
    JVT    = MakeIdentifier('J', 'V', 'T'          ),
    LBM    = MakeIdentifier('L', 'B', 'M'          ),
    LRC    = MakeIdentifier('L', 'R', 'C'          ),
    M2V    = MakeIdentifier('M', '2', 'V'          ),
    MAD    = MakeIdentifier('M', 'A', 'D'          ),
    MJPG   = MakeIdentifier('M', 'J', 'P', 'G'     ),
    MKA    = MakeIdentifier('M', 'K', 'A'          ),
    MKS    = MakeIdentifier('M', 'K', 'S'          ),
    MLV    = MakeIdentifier('M', 'L', 'V'          ),
    MM     = MakeIdentifier('M', 'M'               ),
    MODEL  = MakeIdentifier('M', 'O', 'D', 'E', 'L'),
    MOVIE  = MakeIdentifier('M', 'O', 'V', 'I', 'E'),
    MTV    = MakeIdentifier('M', 'T', 'V'          ),
    MV     = MakeIdentifier('M', 'V'               ),
    MVE    = MakeIdentifier('M', 'V', 'E'          ),
    MVI    = MakeIdentifier('M', 'V', 'I'          ),
    MXG    = MakeIdentifier('M', 'X', 'G'          ),
    NAL    = MakeIdentifier('N', 'A', 'L'          ),
    NSV    = MakeIdentifier('N', 'S', 'V'          ),
    NUV    = MakeIdentifier('N', 'U', 'V'          ),
    OLD    = MakeIdentifier('O', 'L', 'D'          ),
    OSQ    = MakeIdentifier('O', 'S', 'Q'          ),
    PAF    = MakeIdentifier('P', 'A', 'F'          ),
    PCT    = MakeIdentifier('P', 'C', 'T'          ),
    PGM    = MakeIdentifier('P', 'G', 'M'          ),
    PIC    = MakeIdentifier('P', 'I', 'C'          ),
    PIX    = MakeIdentifier('P', 'I', 'X'          ),
    PJS    = MakeIdentifier('P', 'J', 'S'          ),
    PMP    = MakeIdentifier('P', 'M', 'P'          ),
    PNM    = MakeIdentifier('P', 'N', 'M'          ),
    PTX    = MakeIdentifier('P', 'T', 'X'          ),
    PVA    = MakeIdentifier('P', 'V', 'A'          ),
    R3D    = MakeIdentifier('R', '3', 'D'          ),
    RA     = MakeIdentifier('R', 'A'               ),
    RAS    = MakeIdentifier('R', 'A', 'S'          ),
    RAW    = MakeIdentifier('R', 'A', 'W'          ),
    RKA    = MakeIdentifier('R', 'K', 'A'          ),
    RL2    = MakeIdentifier('R', 'L', '2'          ),
    RM     = MakeIdentifier('R', 'M'               ),
    RMHD   = MakeIdentifier('R', 'M', 'H', 'D'     ),
    ROQ    = MakeIdentifier('R', 'O', 'Q'          ),
    RPL    = MakeIdentifier('R', 'P', 'L'          ),
    RSD    = MakeIdentifier('R', 'S', 'D'          ),
    RT     = MakeIdentifier('R', 'T'               ),
    S16    = MakeIdentifier('S', '1', '6'          ),
    SCC    = MakeIdentifier('S', 'C', 'C'          ),
    SDR    = MakeIdentifier('S', 'D', 'R'          ),
    SEQ    = MakeIdentifier('S', 'E', 'Q'          ),
    SGI    = MakeIdentifier('S', 'G', 'I'          ),
    SHQ2   = MakeIdentifier('S', 'H', 'Q', '2'     ),
    SMI    = MakeIdentifier('S', 'M', 'I'          ),
    SMK    = MakeIdentifier('S', 'M', 'K'          ),
    SMV    = MakeIdentifier('S', 'M', 'V'          ),
    SOL    = MakeIdentifier('S', 'O', 'L'          ),
    SRT    = MakeIdentifier('S', 'R', 'T'          ),
    SSA    = MakeIdentifier('S', 'S', 'A'          ),
    STL    = MakeIdentifier('S', 'T', 'L'          ),
    STR    = MakeIdentifier('S', 'T', 'R'          ),
    SUB    = MakeIdentifier('S', 'U', 'B'          ),
    SUN    = MakeIdentifier('S', 'U', 'N'          ),
    SUP    = MakeIdentifier('S', 'U', 'P'          ),
    SW     = MakeIdentifier('S', 'W'               ),
    TGA    = MakeIdentifier('T', 'G', 'A'          ),
    TGQ    = MakeIdentifier('T', 'G', 'Q'          ),
    TGV    = MakeIdentifier('T', 'G', 'V'          ),
    THP    = MakeIdentifier('T', 'H', 'P'          ),
    TMV    = MakeIdentifier('T', 'M', 'V'          ),
    TREC   = MakeIdentifier('T', 'R', 'E', 'C'     ),
    TXD    = MakeIdentifier('T', 'X', 'D'          ),
    VAG    = MakeIdentifier('V', 'A', 'G'          ),
    VB     = MakeIdentifier('V', 'B'               ),
    VID    = MakeIdentifier('V', 'I', 'D'          ),
    VMD    = MakeIdentifier('V', 'M', 'D'          ),
    VOB    = MakeIdentifier('V', 'O', 'B'          ),
    VP6    = MakeIdentifier('V', 'P', '6'          ),
    VP7    = MakeIdentifier('V', 'P', '7'          ),
    VQA    = MakeIdentifier('V', 'Q', 'A'          ),
    VTT    = MakeIdentifier('V', 'T', 'T'          ),
    WTV    = MakeIdentifier('W', 'T', 'V'          ),
    XA     = MakeIdentifier('X', 'A'               ),
    XBM    = MakeIdentifier('X', 'B', 'M'          ),
    XESC   = MakeIdentifier('X', 'E', 'S', 'C'     ),
    XFACE  = MakeIdentifier('X', 'F', 'A', 'C', 'E'),
    YOP    = MakeIdentifier('Y', 'O', 'P'          ),
    ZNM    = MakeIdentifier('Z', 'N', 'M'          ),
};

namespace FileSystem
{

static uint64_t MakeIdentifier(const std::string &path)
{
    if (path.empty())
    {
        return 0;
    }

    const size_t dot = path.find_last_of('.');
    if (dot == std::string::npos || dot == 0 || dot + 1 >= path.size())
    {
        return 0;
    }

    uint64_t id = 0;
    for (auto it = path.rbegin(); it != path.rend() && it.base() > path.begin() + (ptrdiff_t)dot + 1; ++it)
    {
        id = (id << 8) | (uint8_t)std::toupper((unsigned char)*it);
    }

    return id;
}

static inline FileFormat DumpFileId(const std::string &path)
{
    return (FileFormat)MakeIdentifier(path);
}

template <FileFormat T>
inline constexpr bool IsFormat(const std::string &path)
{
    auto id = MakeIdentifier(path);
    return id == uint64_t(T);
}

template <FileFormat T>
inline constexpr bool IsFormat(uint64_t id)
{
    return id == uint64_t(T);
}

template <FileFormat T>
inline constexpr bool IsFormat(FileFormat id)
{
    return id == T;
}

static inline bool Is3DModel(const std::string &path)
{
    auto id = MakeIdentifier(path);

    return IsFormat<FileFormat::OBJ>(id) ||
           IsFormat<FileFormat::FBX>(id) ||
           IsFormat<FileFormat::BLEND>(id) ||
           IsFormat<FileFormat::GLTF>(id);
}

static inline bool IsRawImage(FileFormat id)
{
    return IsFormat<FileFormat::CR2>(id)  ||
	       IsFormat<FileFormat::_3FR>(id) ||
           IsFormat<FileFormat::ARW>(id)  ||
           IsFormat<FileFormat::NEF>(id)  ||
           IsFormat<FileFormat::FFF>(id)  ||
	       IsFormat<FileFormat::_3FR>(id) ||
           IsFormat<FileFormat::RAF>(id)  ||
           IsFormat<FileFormat::RW2>(id)  ||
           IsFormat<FileFormat::DNG>(id);
}

static inline bool IsImage(FileFormat id)
{
    return IsFormat<FileFormat::BMP>(id) ||
           IsFormat<FileFormat::JPEG>(id) ||
           IsFormat<FileFormat::JPG>(id) ||
           IsFormat<FileFormat::JXL>(id) ||
           IsFormat<FileFormat::PNG>(id) ||
           IsFormat<FileFormat::PPM>(id) ||
           IsFormat<FileFormat::HDR>(id) ||
           IsFormat<FileFormat::JFIF>(id) ||
           IsFormat<FileFormat::WEBP>(id) ||
           IsFormat<FileFormat::TIFF>(id) ||
           IsFormat<FileFormat::TIF>(id) ||
           IsFormat<FileFormat::HEIC>(id) ||
           IsFormat<FileFormat::AVIF>(id) ||
           IsFormat<FileFormat::GIF>(id) ||
           IsFormat<FileFormat::CIN>(id) ||
           IsFormat<FileFormat::CINE>(id) ||
           IsFormat<FileFormat::DDS>(id) ||
           IsFormat<FileFormat::DPX>(id) ||
           IsFormat<FileFormat::EXR>(id) ||
           IsFormat<FileFormat::FIT>(id) ||
           IsFormat<FileFormat::FITS>(id) ||
           IsFormat<FileFormat::ILBM>(id) ||
           IsFormat<FileFormat::J2K>(id) ||
           IsFormat<FileFormat::MJPG>(id) ||
           IsFormat<FileFormat::PCT>(id) ||
           IsFormat<FileFormat::PGM>(id) ||
           IsFormat<FileFormat::PIC>(id) ||
           IsFormat<FileFormat::PIX>(id) ||
           IsFormat<FileFormat::PNM>(id) ||
           IsFormat<FileFormat::PSD>(id) ||
           IsFormat<FileFormat::RAS>(id) ||
           IsFormat<FileFormat::SGI>(id) ||
           IsFormat<FileFormat::SUN>(id) ||
           IsFormat<FileFormat::TGA>(id) ||
           IsFormat<FileFormat::XBM>(id) ||
           IsFormat<FileFormat::XFACE>(id) ||
           IsRawImage(id);
}
static inline bool IsImage(uint64_t format)
{
    return IsImage((FileFormat)format);
}

static inline bool IsImage(const std::string &path)
{
    auto id = MakeIdentifier(path);
    return IsImage(id);
}

static inline bool IsVideo(FileFormat id)
{
    return IsFormat<FileFormat::IVF>(id) ||
           IsFormat<FileFormat::OBU>(id) ||
           IsFormat<FileFormat::MP4>(id) ||
           IsFormat<FileFormat::VVC>(id) ||
           IsFormat<FileFormat::H264>(id) ||
           IsFormat<FileFormat::H265>(id) ||
           IsFormat<FileFormat::H266>(id) ||
           IsFormat<FileFormat::_266>(id) ||
           IsFormat<FileFormat::MKV>(id) ||
           IsFormat<FileFormat::M2TS>(id) ||
           IsFormat<FileFormat::TS>(id) ||
           IsFormat<FileFormat::MOV>(id) ||
           IsFormat<FileFormat::WEBM>(id) ||
           IsFormat<FileFormat::BIT>(id) ||
           IsFormat<FileFormat::MXF>(id) ||
           IsFormat<FileFormat::OGV>(id) ||
           IsFormat<FileFormat::OGG>(id) ||
           IsFormat<FileFormat::VC1>(id) ||
           IsFormat<FileFormat::MNG>(id) ||
           IsFormat<FileFormat::QT>(id) ||
           IsFormat<FileFormat::WMV>(id) ||
           IsFormat<FileFormat::RMVB>(id) ||
           IsFormat<FileFormat::ASF>(id) ||
           IsFormat<FileFormat::AMV>(id) ||
           IsFormat<FileFormat::M4V>(id) ||
           IsFormat<FileFormat::MPG>(id) ||
           IsFormat<FileFormat::_3GP>(id) ||
           IsFormat<FileFormat::FLV>(id) ||
           IsFormat<FileFormat::AVI>(id) ||
           IsFormat<FileFormat::_11C>(id) ||
           IsFormat<FileFormat::_264>(id) ||
           IsFormat<FileFormat::_26L>(id) ||
           IsFormat<FileFormat::_302>(id) ||
           IsFormat<FileFormat::_44C>(id) ||
           IsFormat<FileFormat::_4XM>(id) ||
           IsFormat<FileFormat::_5C>(id) ||
           IsFormat<FileFormat::AA3>(id) ||
           IsFormat<FileFormat::ANM>(id) ||
           IsFormat<FileFormat::APC>(id) ||
           IsFormat<FileFormat::AVC>(id) ||
           IsFormat<FileFormat::AVS>(id) ||
           IsFormat<FileFormat::BCSTM>(id) ||
           IsFormat<FileFormat::BFI>(id) ||
           IsFormat<FileFormat::BFSTM>(id) ||
           IsFormat<FileFormat::BIK>(id) ||
           IsFormat<FileFormat::BMV>(id) ||
           IsFormat<FileFormat::BRSTM>(id) ||
           IsFormat<FileFormat::BS>(id) ||
           IsFormat<FileFormat::C93>(id) ||
           IsFormat<FileFormat::CDXL>(id) ||
           IsFormat<FileFormat::CMV>(id) ||
           IsFormat<FileFormat::CPK>(id) ||
           IsFormat<FileFormat::DCT>(id) ||
           IsFormat<FileFormat::DIVX>(id) ||
           IsFormat<FileFormat::DNXHR>(id) ||
           IsFormat<FileFormat::DRC>(id) ||
           IsFormat<FileFormat::DSS>(id) ||
           IsFormat<FileFormat::DXA>(id) ||
           IsFormat<FileFormat::EVC>(id) ||
           IsFormat<FileFormat::FLI>(id) ||
           IsFormat<FileFormat::FMV>(id) ||
           IsFormat<FileFormat::GDV>(id) ||
           IsFormat<FileFormat::H263>(id) ||
           IsFormat<FileFormat::HEVC>(id) ||
           IsFormat<FileFormat::IFF>(id) ||
           IsFormat<FileFormat::JLS>(id) ||
           IsFormat<FileFormat::LBM>(id) ||
           IsFormat<FileFormat::M2V>(id) ||
           IsFormat<FileFormat::MKA>(id) ||
           IsFormat<FileFormat::MKS>(id) ||
           IsFormat<FileFormat::MLV>(id) ||
           IsFormat<FileFormat::MOVIE>(id) ||
           IsFormat<FileFormat::MTV>(id) ||
           IsFormat<FileFormat::MV>(id) ||
           IsFormat<FileFormat::MVE>(id) ||
           IsFormat<FileFormat::MVI>(id) ||
           IsFormat<FileFormat::MXG>(id) ||
           IsFormat<FileFormat::NSV>(id) ||
           IsFormat<FileFormat::NUV>(id) ||
           IsFormat<FileFormat::PAF>(id) ||
           IsFormat<FileFormat::PMP>(id) ||
           IsFormat<FileFormat::PTX>(id) ||
           IsFormat<FileFormat::PVA>(id) ||
           IsFormat<FileFormat::R3D>(id) ||
           IsFormat<FileFormat::RA>(id) ||
           IsFormat<FileFormat::RAW>(id) ||
           IsFormat<FileFormat::RKA>(id) ||
           IsFormat<FileFormat::RL2>(id) ||
           IsFormat<FileFormat::RM>(id) ||
           IsFormat<FileFormat::RMHD>(id) ||
           IsFormat<FileFormat::ROQ>(id) ||
           IsFormat<FileFormat::RPL>(id) ||
           IsFormat<FileFormat::RSD>(id) ||
           IsFormat<FileFormat::S16>(id) ||
           IsFormat<FileFormat::SEQ>(id) ||
           IsFormat<FileFormat::SHQ2>(id) ||
           IsFormat<FileFormat::SMK>(id) ||
           IsFormat<FileFormat::SMV>(id) ||
           IsFormat<FileFormat::SOL>(id) ||
           IsFormat<FileFormat::STR>(id) ||
           IsFormat<FileFormat::TGQ>(id) ||
           IsFormat<FileFormat::TGV>(id) ||
           IsFormat<FileFormat::THP>(id) ||
           IsFormat<FileFormat::TMV>(id) ||
           IsFormat<FileFormat::VAG>(id) ||
           IsFormat<FileFormat::VB>(id) ||
           IsFormat<FileFormat::VID>(id) ||
           IsFormat<FileFormat::VMD>(id) ||
           IsFormat<FileFormat::VOB>(id) ||
           IsFormat<FileFormat::VP6>(id) ||
           IsFormat<FileFormat::VP7>(id) ||
           IsFormat<FileFormat::VQA>(id) ||
           IsFormat<FileFormat::WTV>(id) ||
           IsFormat<FileFormat::XA>(id) ||
           IsFormat<FileFormat::YOP>(id);
}
static inline bool IsVideo(uint64_t format)
{
	return IsVideo((FileFormat) format);
}

static inline bool IsAudio(FileFormat id)
{
	return IsFormat<FileFormat::AAC>(id) ||
	       IsFormat<FileFormat::MP3>(id) ||
	       IsFormat<FileFormat::FLAC>(id) ||
	       IsFormat<FileFormat::WAV>(id) ||
	       IsFormat<FileFormat::OPUS>(id) ||
	       IsFormat<FileFormat::AC3>(id) ||
	       IsFormat<FileFormat::ADTS>(id) ||
	       IsFormat<FileFormat::AEA>(id) ||
	       IsFormat<FileFormat::AMR>(id) ||
	       IsFormat<FileFormat::APE>(id) ||
	       IsFormat<FileFormat::APM>(id) ||
	       IsFormat<FileFormat::AWB>(id) ||
	       IsFormat<FileFormat::CAF>(id) ||
	       IsFormat<FileFormat::DFF>(id) ||
	       IsFormat<FileFormat::DTS>(id) ||
	       IsFormat<FileFormat::DTSHD>(id) ||
	       IsFormat<FileFormat::EAC3>(id) ||
	       IsFormat<FileFormat::F32>(id) ||
	       IsFormat<FileFormat::G722>(id) ||
	       IsFormat<FileFormat::IAMF>(id) ||
	       IsFormat<FileFormat::M4A>(id) ||
	       IsFormat<FileFormat::MLP>(id) ||
	       IsFormat<FileFormat::MPC>(id) ||
	       IsFormat<FileFormat::NIST>(id) ||
	       IsFormat<FileFormat::OGG>(id) ||
	       IsFormat<FileFormat::OMA>(id) ||
	       IsFormat<FileFormat::PCM>(id) ||
	       IsFormat<FileFormat::QCP>(id) ||
	       IsFormat<FileFormat::QOA>(id) ||
	       IsFormat<FileFormat::RCV>(id) ||
	       IsFormat<FileFormat::SHN>(id) ||
	       IsFormat<FileFormat::TAK>(id) ||
	       IsFormat<FileFormat::TCO>(id) ||
	       IsFormat<FileFormat::THD>(id) ||
	       IsFormat<FileFormat::TTA>(id) ||
	       IsFormat<FileFormat::TUN>(id) ||
	       IsFormat<FileFormat::VOC>(id) ||
	       IsFormat<FileFormat::VQF>(id) ||
	       IsFormat<FileFormat::W64>(id) ||
	       IsFormat<FileFormat::WMA>(id) ||
	       IsFormat<FileFormat::WV>(id) ||
	       IsFormat<FileFormat::WVE>(id) ||
	       IsFormat<FileFormat::XWMA>(id);
}
static inline bool IsAudio(uint64_t format)
{
	return IsAudio((FileFormat) format);
}

static inline bool IsVideo(const std::string &path)
{
    auto id = MakeIdentifier(path);
    return IsVideo(id);
}

static inline bool IsAudio(const std::string &path)
{
	auto id = MakeIdentifier(path);
	return IsAudio(id);
}

static FileType GetFileType(const std::string &path)
{
    auto id = DumpFileId(path);

    if (IsRawImage(id))
    {
        return FileType::RAW;
    }

    if (IsAudio(id))
    {
		return FileType::Audio;
    }

#define CASE(X) case FileFormat::##X: return FileType::##X;
    switch (id)
    {
    case FileFormat::BLEND:
    case FileFormat::GLTF:
    case FileFormat::FBX:
    case FileFormat::OBJ:
        return FileType::OBJ;

    case FileFormat::IVF:
    case FileFormat::H264:
    case FileFormat::H265:
    case FileFormat::MKV:
    case FileFormat::TS:
    case FileFormat::M2TS:
    case FileFormat::WEBM:
        return FileType::Video;

    CASE(CPP )
    CASE(EXE )
    CASE(BIN )
    CASE(MP4 )
    CASE(MOV )
    CASE(JSON)
    CASE(PDF )
    CASE(HTML)
    CASE(AVI )
    CASE(BMP )
    CASE(DLL )
    CASE(DAT )
    CASE(DOC )
    CASE(GIF )
    CASE(JPG )
    CASE(JS  )
    CASE(PNG )
    CASE(PPT )
    CASE(PSD )
    CASE(SQL )
    CASE(TIF )
    CASE(TXT )
    CASE(XML )
    CASE(ZIP )
	CASE(WEBP)
	CASE(JXL )
	CASE(TIFF)
    default:
        return FileType::RegularFile;
    }
#undef CASE
}

static inline std::vector<uint8_t> ReadBinary(const String &filename, uint32_t align = sizeof(void*))
{
    std::vector<uint8_t> buffer{};
    Stream stream{ filename, Stream::Mode::Read };
    if (!stream.Readable())
    {
        LOG::WARN("Unable to open {0}", filename);
        return buffer;
    }

    buffer.resize(stream.GetSize());

    stream.Read(buffer.data(), buffer.size());

    return buffer;
}

static inline std::string ReadString(const String &filename)
{
    std::string buffer{};
    Stream stream{ filename, Stream::Mode::Read };
    if (!stream.Readable())
    {
        LOG::WARN("Unable to open {0}", filename);
        return buffer;
    }

    buffer.resize(stream.GetSize());

    stream.Read(buffer.data(), buffer.size());

    return buffer;
}

static std::string ExtractFileName(const std::string &path)
{
    auto lastSlash = path.find_last_of("/\\");
    auto lastDot   = path.rfind('.');

    lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;

    return path.substr(lastSlash, std::min(lastDot, path.size()) - lastSlash);
}

struct FileAttributeString
{
	bool hasData;
	String name;
	String size;
	String creationTime;
	String lastWriteTime;
	String lastAccessTime;
};

struct FileAttribute
{
    size_t size = 0;
    std::filesystem::file_time_type creationTime;
    std::filesystem::file_time_type lastWriteTime;
    std::filesystem::file_time_type lastAccessTime;

    static String FileTimeToString(const std::filesystem::file_time_type &ft, const char* fmt = "%Y/%m/%d, %H:%M:%S")
    {
        if (ft == std::filesystem::file_time_type{}) return "N/A";
		auto sctp = std::chrono::system_clock::time_point(duration_cast<std::chrono::system_clock::duration>(ft.time_since_epoch()));
		std::time_t cftime = std::chrono::system_clock::to_time_t(sctp);
        std::tm tm;

#ifdef _WIN32
		localtime_s(&tm, &cftime);
#else
		localtime_r(&cftime, &tm);
#endif

        char buf[32];
		std::strftime(buf, sizeof(buf), fmt, &tm);
        return buf;
    }

    void ToString(FileAttributeString &attribute) const
    {
		float kb = size / 1024.0f;
		float mb = kb / 1024.0f;

        char s[128];
        if (mb >= 1.0f)
		{
			sprintf(&s[0], "%.2f", mb);
			attribute.size = s + String(" MB");
        }
		else if (kb >= 1.0f)
		{
			sprintf(&s[0], "%.2f", kb);
			attribute.size = s + String(" KB");
        }
        else
        {
			attribute.size = std::to_string(size) + String(" ") + "Byte(s)";
        }

		attribute.hasData = true;
		attribute.creationTime   = FileTimeToString(creationTime);
		attribute.lastWriteTime  = FileTimeToString(lastWriteTime);
		attribute.lastAccessTime = FileTimeToString(lastAccessTime);
    }
};

class Path : public std::filesystem::path
{
public:
    using Super = std::filesystem::path;

public:
    Path() :
        Super{}
    {

    }

    Path(const Super &path) :
        Super{ path }
    {

    }

    Path(Super &&path) :
        Super{ std::move(path) }
    {

    }

    Path(const char *path) :
        Super{ path }
    {

    }

    Path(const std::string &path) :
        Super{ path }
    {

    }

    Path(const wchar_t *path) :
        Super{ path }
    {

    }

    Path(const std::wstring &path) :
        Super{ path }
    {

    }

    Path(const String &path) :
        Super{ (const std::u8string &)path }
    {

    }

    Path(const std::u8string_view &view) :
        Super{ view }
    {

    }

    operator bool() const
    {
        return std::filesystem::exists(*this);
    }

    static Path Current()
    {
        return std::move(std::filesystem::current_path());
    }

    operator String() const
    {
        return u8string();
    }

    Path Parent() const
    {
        return parent_path();
    }

    size_t Length() const
    {
        return string().size();
    }

    bool Exists() const
    {
        return std::filesystem::exists(*this);
    }

    bool IsDirectory() const
    {
        return std::filesystem::is_directory(*this);
    }

    void GetAttribute(FileAttribute &attribute) const;
};

static inline bool CreateDirectory(const FileSystem::Path &path)
{
    return std::filesystem::create_directory(path);
}

static inline bool CreateDirectories(const FileSystem::Path &path)
{
	return std::filesystem::create_directories(path);
}

std::string_view ParseFileName(const String &path);

struct DirectoryEntry
{
    String path;

    FileType type;

    std::string_view fileName;

	std::vector<DirectoryEntry> subdirectories;

    FileFlagBits flags;

    int star;

    int color;

    uint32_t id;

    bool isEmpty;

    /** Seconds since Unix epoch (local interpretation for UI buckets). 0 = unknown. */
    int64_t creationUnixSec;

    int64_t lastWriteUnixSec;

    uint64_t fileSize;

    DirectoryEntry(String &&_path, FileType type) :
	    path{std::move(_path)},
	    type{type},
	    fileName{ParseFileName(path)},
	    subdirectories{},
	    star{},
        color{},
	    id{},
	    isEmpty{true},
	    creationUnixSec{},
	    lastWriteUnixSec{},
	    fileSize{},
	    flags{}
	{

    }

    DirectoryEntry(const String &_path, FileType type) :
        path{ _path },
        type{ type },
        fileName{ ParseFileName(path) },
	    subdirectories{},
	    star{},
        color{},
	    id{},
        isEmpty{ true },
	    creationUnixSec{},
	    lastWriteUnixSec{},
	    fileSize{},
        flags{}
    {

    }

    DirectoryEntry() :
        path{},
        type{},
	    fileName{path.c_str()},
	    subdirectories{},
	    star{},
        color{},
	    id{},
        isEmpty{},
	    creationUnixSec{},
	    lastWriteUnixSec{},
	    fileSize{},
        flags{}
    {

    }

    DirectoryEntry(const DirectoryEntry &other) :
        path{ other.path },
        type{ other.type },
	    fileName{path.c_str() + path.size() - other.fileName.size()},
	    subdirectories{other.subdirectories},
	    star{ other.star },
	    color{ other.color },
	    id{other.id},
        isEmpty{ other.isEmpty },
	    creationUnixSec{ other.creationUnixSec },
	    lastWriteUnixSec{ other.lastWriteUnixSec },
	    fileSize{ other.fileSize },
        flags{other.flags}
    {

    }

    DirectoryEntry(DirectoryEntry &&other) :
        DirectoryEntry{}
    {
        other.Swap(*this);
    }

    ~DirectoryEntry()
    {

    }

    DirectoryEntry &operator=(const DirectoryEntry &other)
    {
        DirectoryEntry(other).Swap(*this);
        return *this;
    }

    DirectoryEntry &operator=(DirectoryEntry &&other)
    {
        DirectoryEntry(std::move(other)).Swap(*this);
        return *this;
    }

    const char *GetFileName() const
    {
        return fileName.empty() ? "" : (const char *)fileName.data();
    }

    bool IsDirectory() const
    {
        return type == FileType::Directory ||
               type == FileType::Volumn    ||
               type == FileType::Desktop;
    }

    bool IsRegularFile() const
    {
        return type == FileType::RegularFile;
    }

    bool IsEmpty() const
    {
        return isEmpty;
    }

    void SetIsEmpty(bool value)
    {
        isEmpty = value;
    }

    void Swap(DirectoryEntry &other)
    {
		int lPos = fileName.data() - path.c_str();
        int lSize = path.size() - lPos;

        int rPos = other.fileName.data() - other.path.c_str();
        int rSize = other.path.size() - rPos;

        path.Swap(other.path);
		std::swap(type,           other.type          );
		std::swap(subdirectories, other.subdirectories);
		std::swap(star,           other.star          );
        std::swap(color,          other.color         );
		std::swap(id,             other.id            );
		std::swap(isEmpty,        other.isEmpty       );
		std::swap(creationUnixSec, other.creationUnixSec);
		std::swap(lastWriteUnixSec, other.lastWriteUnixSec);
		std::swap(fileSize,       other.fileSize      );
        std::swap(flags,          other.flags         );

        fileName = {path.c_str() + rPos, size_t(rSize)};
		other.fileName = {other.path.c_str() + lPos, size_t(lSize)};
    }
};

bool HasSubdirectory(const Path &path);

void ListDirectory(const Path &path, std::vector<DirectoryEntry> &directories, FileType filter = FileType::Directory | FileType::RegularFile);

static inline bool Exists(const String &path)
{
    return std::filesystem::exists(path.GetU8String());
}

static inline std::string Join(const std::string &lpath, const std::string &rpath)
{
    /* Temporary */
    size_t pos = 0;
    while (rpath[pos] == '/' || rpath[pos] == '\\')
    {
        pos++;
    }
    if (lpath.back() == '/' || lpath.back() == '\\')
    {
        return lpath + std::string{ rpath.data() + pos };
    }
    return lpath + std::string{ '/' } + rpath;
}

}

using Path          = FileSystem::Path;
using FileAttribute = FileSystem::FileAttribute;

}
