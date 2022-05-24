#pragma once

#include <filesystem>

#include "Core.h"
#include "Stream.h"

namespace Immortal
{

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
    Binary,
    Text
};

enum class FileFormat : uint64_t
{
    BLEND = MakeIdentifier('B', 'L', 'E', 'N', 'D'),
    GLTF  = MakeIdentifier('G', 'L', 'T', 'F'     ),
    BMP   = MakeIdentifier('B', 'M', 'P'          ),
    FBX   = MakeIdentifier('F', 'B', 'X'          ),
    OBJ   = MakeIdentifier('O', 'B', 'J'          ),
    PNG   = MakeIdentifier('P', 'N', 'G'          ),
    PPM   = MakeIdentifier('P', 'P', 'M'          ),
    JFIF  = MakeIdentifier('J', 'F', 'I', 'F'     ),
    JPG   = MakeIdentifier('J', 'P', 'G'          ),
    JPEG  = MakeIdentifier('J', 'P', 'E', 'G'     ),
    IVF   = MakeIdentifier('I', 'V', 'F'          ),
    IML   = MakeIdentifier('I', 'M', 'L'          ),
    HDR   = MakeIdentifier('H', 'D', 'R'          ),
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
    return id == U64(T);
}

template <FileFormat T>
inline constexpr bool IsFormat(uint64_t id)
{
    return id == U64(T);
}

static inline bool Is3DModel(const std::string &path)
{
    auto id = MakeIdentifier(path);

    return IsFormat<FileFormat::OBJ>(id) ||
           IsFormat<FileFormat::FBX>(id) ||
           IsFormat<FileFormat::BLEND>(id) || 
           IsFormat<FileFormat::GLTF>(id);
}

static inline bool IsImage(const std::string &path)
{
    auto id = MakeIdentifier(path);

    return IsFormat<FileFormat::BMP>(id)  ||
           IsFormat<FileFormat::JPEG>(id) ||
           IsFormat<FileFormat::JPG>(id)  ||
           IsFormat<FileFormat::PNG>(id)  ||
           IsFormat<FileFormat::PPM>(id)  ||
           IsFormat<FileFormat::HDR>(id)  ||
           IsFormat<FileFormat::JFIF>(id);
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

static void MakeDirectory(const std::string &path)
{
    std::filesystem::create_directory(path);
}

namespace Path
{

static bool Exsits(const std::string &path)
{
    return std::filesystem::exists(path);
}

static std::string Join(const std::string &lpath, const std::string &rpath)
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

};

}
