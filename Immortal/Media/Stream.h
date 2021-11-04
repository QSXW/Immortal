#pragma

#include <iostream>
#include <string>
#include <cstdint>

#include "Types.h"

namespace sl
{

class Stream
{
public:
    enum class Mode : uint32_t
    {
        Read  = 0x002b6272,
        Write = 0x002b6277
    };

public:
    Stream()
    {
    
    }

    Stream(const std::string &filepath, Mode mode) :
        mode{ mode }
    {
        fp = fopen(filepath.c_str(), reinterpret_cast<const char *>(&mode));
        if (!fp)
        {
            error = Status::UNABLE_TO_OEPN_FILE;
            return;
        }
        fileSize = GetFileSize(fp);
    }

    ~Stream()
    {
        if (!fp)
        {
            return;
        }
        fclose(fp);
    }

    bool ReOpen(const std::string &filepath, Mode mode)
    {
        if (fp)
        {
            fclose(fp);
            fp = nullptr;
        }
    }

    bool Readable()
    {
        return !!fp && mode == Mode::Read;
    }

    bool Writable()
    {
        return !!fp && mode == Mode::Write;
    }

    size_t Size()
    {
        return fileSize;
    }

    size_t Read(void *dst, size_t size, size_t count = 1)
    {
        return fread(dst, size, count, fp);
    }

    template <size_t size, size_t count = 1>
    size_t Write(const void *src)
    {
        return fwrite(src, size, count, fp);
    }

    size_t Write(const void *src, size_t size, size_t count = 1)
    {
        return fwrite(src, size, count, fp);
    }

    size_t Pos()
    {
        return ftell(fp);
    }

    int Locate(size_t pos)
    {
        return fseek(fp, pos, SEEK_SET);
    }

    int Skip(size_t offset)
    {
        return fseek(fp, offset, SEEK_CUR);
    }

public:
    static inline size_t GetFileSize(FILE *fp)
    {
        fseek(fp, 0, SEEK_END);
        auto size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        return size;
    }

private:
    Status error{ Status::SUCCEED };

    Mode mode{ 0 };

    FILE *fp{ nullptr };

    size_t fileSize{ 0 };
};

}
