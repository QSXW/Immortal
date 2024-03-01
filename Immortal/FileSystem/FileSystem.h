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
    Directory,
    RegularFile,
    Picture,
    Video,
    Audio,
    CPP,
    Object,
    Executable,
    Binary,
    Text,
};

enum class FileFormat : uint64_t
{
    /** 3D Model formats supported by Assimp */
    BLEND = MakeIdentifier('B', 'L', 'E', 'N', 'D'),
    GLTF  = MakeIdentifier('G', 'L', 'T', 'F'     ),
    FBX   = MakeIdentifier('F', 'B', 'X'          ),
    OBJ   = MakeIdentifier('O', 'B', 'J'          ),

    /** Audio formats */
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
    HDR   = MakeIdentifier('H', 'D', 'R'     ),
    ARW   = MakeIdentifier('A', 'R', 'W'     ),
    NEF   = MakeIdentifier('N', 'E', 'F'     ),
    CR2   = MakeIdentifier('C', 'R', '2'     ),
    CR3   = MakeIdentifier('C', 'R', '3'     ),
    AVIF  = MakeIdentifier('A', 'V', 'I', 'F'),

    /** Video file format extensions */
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

    /** Immortal Scene */
    IML = MakeIdentifier('I', 'M', 'L'),

    CPP = MakeIdentifier('C', 'P', 'P'),

    EXE = MakeIdentifier('E', 'X', 'E'),

};

namespace FileSystem
{

static uint64_t MakeIdentifier(const std::string &path)
{
    uint64_t id = 0;

    size_t i = path.size() - 1;

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

static inline bool Is3DModel(const std::string &path)
{
    auto id = MakeIdentifier(path);

    return IsFormat<FileFormat::OBJ>(id) ||
           IsFormat<FileFormat::FBX>(id) ||
           IsFormat<FileFormat::BLEND>(id) ||
           IsFormat<FileFormat::GLTF>(id);
}

static inline bool IsImage(uint64_t id)
{
    return IsFormat<FileFormat::BMP>(id)  ||
           IsFormat<FileFormat::JPEG>(id) ||
           IsFormat<FileFormat::JPG>(id)  ||
           IsFormat<FileFormat::PNG>(id)  ||
           IsFormat<FileFormat::PPM>(id)  ||
           IsFormat<FileFormat::HDR>(id)  ||
           IsFormat<FileFormat::ARW>(id)  ||
           IsFormat<FileFormat::NEF>(id)  ||
           IsFormat<FileFormat::CR2>(id)  ||
           IsFormat<FileFormat::JFIF>(id);
}

static inline bool IsImage(const std::string &path)
{
    auto id = MakeIdentifier(path);
    return IsImage(id);
}

static inline bool IsVideo(uint64_t id)
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
	       IsFormat<FileFormat::WEBM>(id) ||
	       IsFormat<FileFormat::AVIF>(id);
}

static inline bool IsVideo(const std::string &path)
{
    auto id = MakeIdentifier(path);
    return IsVideo(id);
}

static FileType GetFileType(const std::string &path)
{
    auto id = DumpFileId(path);
    switch (id)
    {
    case FileFormat::BLEND:
    case FileFormat::GLTF:
    case FileFormat::FBX:
    case FileFormat::OBJ:
        return FileType::Object;

    case FileFormat::WAV:
    case FileFormat::FLAC:
    case FileFormat::MP3:
        return FileType::Audio;

    case FileFormat::BMP:
    case FileFormat::PNG:
    case FileFormat::PPM:
    case FileFormat::JFIF:
    case FileFormat::JPG:
    case FileFormat::JPEG:
    case FileFormat::HDR:
    case FileFormat::ARW:
    case FileFormat::NEF:
    case FileFormat::CR2:
    case FileFormat::CR3:
        return FileType::Picture;

    case FileFormat::IVF:
    case FileFormat::MP4:
    case FileFormat::H264:
    case FileFormat::H265:
    case FileFormat::MKV:
    case FileFormat::TS:
    case FileFormat::MOV:
    case FileFormat::M2TS:
    case FileFormat::WEBM:
        return FileType::Video;

    case FileFormat::CPP:
        return FileType::CPP;

    case FileFormat::EXE:
        return FileType::Executable;

    default:
        return FileType::RegularFile;
    }
}

static inline std::vector<uint8_t> ReadBinary(const std::string &filename, uint32_t align = sizeof(void*))
{
    std::vector<uint8_t> buffer{};
    Stream stream{ filename, Stream::Mode::Read };
    if (!stream.Readable())
    {
        LOG::WARN("Unable to open {0}", filename);
        return buffer;
    }

    buffer.resize(stream.Size());

    stream.Read(buffer.data(), buffer.size());

    return buffer;
}

static inline std::string ReadString(const std::string &filename)
{
    std::string buffer{};
    Stream stream{ filename, Stream::Mode::Read };
    if (!stream.Readable())
    {
        LOG::WARN("Unable to open {0}", filename);
        return buffer;
    }

    buffer.resize(stream.Size());

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

static inline bool CreateDirectory(const std::string &path)
{
    return std::filesystem::create_directory(path);
}

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

    Path(const std::wstring &path) :
        Super{ path }
    {

    }

    Path(const String &path) :
        Super{ (const std::string &)path }
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

    Path Parent() const
    {
        return parent_path();
    }

    size_t Length() const
    {
        return string().size();
    }
};

struct DirectoryEntry
{
    String path;

    FileType type;

    bool IsDirectory() const
    {
        return type == FileType::Directory;
    }

    bool IsRegularFile() const
    {
        return type == FileType::RegularFile;
    }
};

static inline void List(const Path &path, std::vector<DirectoryEntry> &directories)
{
    for (auto &directory : std::filesystem::directory_iterator(path))
    {
        DirectoryEntry entry;
        entry.path = directory.path().string();

        if (directory.is_directory())
        {
            entry.type = FileType::Directory;
        }
        else if (directory.is_regular_file())
        {
            entry.type = FileType::RegularFile;
        }
        directories.emplace_back(std::move(entry));
    }
}

static inline bool Exists(const std::string &path)
{
    return std::filesystem::exists(path);
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

}
