#pragma once

#include <filesystem>

#include "Core.h"
#include "Stream.h"
#include "String/IString.h"
#include "String/LanguageSettings.h"
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
	AAC  = MakeIdentifier('A', 'A', 'C'     ),
    WAV  = MakeIdentifier('W', 'A', 'V'     ),
    FLAC = MakeIdentifier('F', 'L', 'A', 'C'),
    MP3  = MakeIdentifier('M', 'P', '3'     ),

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
    AVI   = MakeIdentifier('A', 'V', 'I'    ),
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
};

namespace FileSystem
{

static uint64_t MakeIdentifier(const std::string &path)
{
    uint64_t id = 0;
    if (path.empty())
    {
		return 0;
    }
    int64_t i = path.size() - 1;

    id |= std::toupper(path[i--]);
    while (i && path[i] != '.')
    {
        id = (id << 8) | std::toupper(path[i--]);
    }

    return path[i] == '.' ? id : 0;
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
    return IsFormat<FileFormat::BMP>(id)  ||
           IsFormat<FileFormat::JPEG>(id) ||
           IsFormat<FileFormat::JPG>(id)  ||
	       IsFormat<FileFormat::JXL>(id) ||
           IsFormat<FileFormat::PNG>(id)  ||
           IsFormat<FileFormat::PPM>(id)  ||
           IsFormat<FileFormat::HDR>(id)  ||
           IsFormat<FileFormat::JFIF>(id) ||
	       IsFormat<FileFormat::WEBP>(id) ||
	       IsFormat<FileFormat::TIFF>(id) ||
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
    return IsFormat<FileFormat::IVF>(id)  ||
           IsFormat<FileFormat::MP4>(id)  ||
           IsFormat<FileFormat::VVC>(id)  ||
           IsFormat<FileFormat::H264>(id) ||
           IsFormat<FileFormat::H265>(id) ||
           IsFormat<FileFormat::H266>(id) ||
           IsFormat<FileFormat::_266>(id) ||
           IsFormat<FileFormat::MKV>(id)  ||
           IsFormat<FileFormat::M2TS>(id) ||
           IsFormat<FileFormat::TS>(id)   ||
           IsFormat<FileFormat::MOV>(id)  ||
           IsFormat<FileFormat::M2TS>(id) ||
           IsFormat<FileFormat::WEBM>(id) ||
           IsFormat<FileFormat::AVIF>(id) ||
           IsFormat<FileFormat::BIT>(id)  ||
           IsFormat<FileFormat::GIF>(id)  ||
	       IsFormat<FileFormat::MXF>(id)  ||
           IsFormat<FileFormat::HEIC>(id);
}

static inline bool IsVideo(uint64_t format)
{
	return IsVideo((FileFormat) format);
}

static inline bool IsAudio(FileFormat id)
{
	return IsFormat<FileFormat::AAC>(id)  ||
	       IsFormat<FileFormat::MP3>(id)  ||
           IsFormat<FileFormat::FLAC>(id) ||
	       IsFormat<FileFormat::WAV>(id);
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

#define CASE(X) case FileFormat::##X: return FileType::##X;
    switch (id)
    {
    case FileFormat::BLEND:
    case FileFormat::GLTF:
    case FileFormat::FBX:
    case FileFormat::OBJ:
        return FileType::OBJ;

    case FileFormat::WAV:
    case FileFormat::FLAC:
    case FileFormat::MP3:
        return FileType::Audio;

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
			attribute.size = std::to_string(size) + String(" ") + Translator::Translate("Byte(s)");
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

    uint32_t id;

    bool isEmpty;

    DirectoryEntry(String &&_path, FileType type) :
	    path{std::move(_path)},
	    type{type},
	    fileName{ParseFileName(path)},
	    subdirectories{},
	    star{},
	    id{},
	    isEmpty{true},
	    flags{}
	{

    }

    DirectoryEntry(const String &_path, FileType type) :
        path{ _path },
        type{ type },
        fileName{ ParseFileName(path) },
	    subdirectories{},
	    star{},
	    id{},
        isEmpty{ true },
        flags{}
    {

    }

    DirectoryEntry() :
        path{},
        type{},
	    fileName{path.c_str()},
	    subdirectories{},
	    star{},
	    id{},
        isEmpty{},
        flags{}
    {

    }

    DirectoryEntry(const DirectoryEntry &other) :
        path{ other.path },
        type{ other.type },
	    fileName{path.c_str() + path.size() - other.fileName.size()},
	    subdirectories{other.subdirectories},
	    star{ other.star },
	    id{other.id},
        isEmpty{ other.isEmpty },
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
        return type == FileType::Directory;
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
		std::swap(id,             other.id            );
        std::swap(isEmpty,        other.isEmpty       );

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
