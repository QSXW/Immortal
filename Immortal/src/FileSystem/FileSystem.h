#pragma once

#include "ImmortalCore.h"
#include "Stream.h"

namespace Immortal
{

enum class FileType
{
    Binary,
    Text
};

class FileSystem
{
public:
    template <class T>
    static T Read(const std::string &filename)
    {
        T buffer{};
        Stream stream{ filename.c_str(), Stream::Mode::Read };
        if (!stream.Readable())
        {
            LOG::WARN("Unable to open {0}", filename.c_str());
            return buffer;
        }
        
        buffer.resize(stream.Size());
        stream.Read(buffer.data(), buffer.size());

        return buffer;
    }

    static std::vector<uint8_t> ReadBinary(const std::string &filename)
    {
        return Read<std::vector<uint8_t>>(filename);
    }

    static std::string ReadString(const std::string &filename)
    {
        return Read<std::string>(filename);
    }
};

}
