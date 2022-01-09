#pragma once

#include <filesystem>
#include "Core.h"
#include "Media/Stream.h"

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
    BMP  = MakeIdentifier('.', 'b', 'm', 'p'),
    FBX  = MakeIdentifier('.', 'f', 'b', 'x'),
    OBJ  = MakeIdentifier('.', 'o', 'b', 'j'),
    JPG  = MakeIdentifier('.', 'j', 'p', 'g'),
    JPEG = MakeIdentifier('.', 'j', 'p', 'e', 'g')
};

namespace FileSystem
{

template <FileFormat T>
bool IsFormat(const std::string &path)
{
    auto p = path.c_str() + path.size();
    return U64(T) == (uint64_t)*(uint32_t *)(p - 4);
}

static inline std::vector<uint8_t> ReadBinary(const std::string &filename)
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
    return lpath + rpath;
}

}

};

}
