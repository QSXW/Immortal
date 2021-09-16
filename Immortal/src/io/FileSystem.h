#pragma once

#include "ImmortalCore.h"

namespace Immortal
{

enum class FileType
{
    Binary,
    Text
};

using Buffer = std::vector<uint8_t>;

class FileSystem
{
public:
    static Buffer ReadBinary(const std::string &filename)
    {
        Buffer buffer{};
        FILE *fp = fopen(filename.c_str(), "rb+");
        if (!fp)
        {
            LOG::WARN("Unable to open {0}", filename.c_str());
            return buffer;
        }

        fseek(fp, 0, SEEK_END);
        size_t size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        
        buffer.resize(size);
        fread(buffer.data(), buffer.size(), 1, fp);

        return buffer;
    }
};

}
