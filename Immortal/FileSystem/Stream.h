#pragma once

#include <iostream>
#include <string>
#include <cstdint>

#include "Core.h"
#include "String/IString.h"

namespace Immortal
{

class Stream
{
public:
    enum class Mode : uint32_t
    {
        Read  = 0x002b6272,
        Write = 0x002b6277
    };

    enum class Type
    {
        Binary,
        Text
    };

public:
    Stream(Mode mode);

    Stream(const String &filepath, Mode mode);

    ~Stream();

    bool Open(const String &path);

    int Close();

    size_t GetSize();

    size_t Read(void *dst, size_t size, size_t count = 1);

    size_t Write(const void *src, size_t size, size_t count = 1);

    size_t Tell() const;

    int Locate(size_t pos);

    int Skip(size_t offset);

public:
    bool Readable()
    {
        return !!handle && mode == Mode::Read;
    }

    bool Writable()
    {
        return !!handle && mode == Mode::Write;
    }

    template <class T>
    size_t Read(T &contatiner)
    {
		size_t size = GetSize();
		contatiner.resize(size / sizeof(contatiner[0]));
		return Read(contatiner.data(), size);
    }

    template <size_t size, size_t count = 1>
    size_t Write(const void *src)
    {
        return Write(src, size, count);
    }

    template <class T>
    size_t Write(const T &src)
    {
        return Write(src.data(), src.size());
    }

    const String &GetFilePath() const
    {
        return filepath;
    }

protected:
    void *handle;

    String filepath;

    Mode mode;
};

using StreamMode = Stream::Mode;

inline Stream::Stream(Mode mode) :
    handle{},
    filepath{},
    mode{mode}
{
}

inline Stream::Stream(const String &filepath, Mode mode) :
    handle{},
    filepath{filepath},
    mode{mode}
{
	Open(filepath);
}

inline Stream::~Stream()
{
	Close();
	handle = nullptr;
}

#ifdef _WIN32
inline bool Stream::Open(const String &path)
{
	filepath = path;

    if (mode == Mode::Read)
    {
		handle = CreateFileA(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    }
	else if (mode == Mode::Write)
	{
		handle = CreateFileA(path.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
	}

	return !!handle;
}

inline int Stream::Close()
{
	if (handle)
	{
		CloseHandle(handle);
		handle = nullptr;
	}

	return 0;
}

inline size_t Stream::GetSize()
{
	LARGE_INTEGER fileSize = {};
	return GetFileSizeEx(handle, &fileSize) ? fileSize.QuadPart : 0;
}

inline size_t Stream::Read(void *dst, size_t size, size_t count)
{
	DWORD read = 0;
	return ReadFile(handle, dst, size, &read, nullptr) ? read : 0;
}

inline size_t Stream::Write(const void *src, size_t size, size_t count)
{
	DWORD written = 0;
	return WriteFile(handle, src, size, &written, nullptr) ? written : 0;
}

inline size_t Stream::Tell() const
{
	LARGE_INTEGER pos = {};
	return SetFilePointerEx(handle, {}, &pos, FILE_CURRENT) ? pos.QuadPart : 0;
}

inline int Stream::Locate(size_t position)
{
	LARGE_INTEGER pos = {};
	return SetFilePointerEx(handle, {.QuadPart = (LONGLONG)position}, &pos, FILE_END) ? pos.QuadPart : 0;
}

inline int Stream::Skip(size_t offset)
{
	LARGE_INTEGER pos = {};
	return SetFilePointerEx(handle, {.QuadPart = (LONGLONG) offset}, &pos, FILE_CURRENT) ? pos.QuadPart : 0;
}


#else

inline bool Stream::Open(const String &path)
{
    filepath = path;
    handle = (void *)fopen(filepath.c_str(), reinterpret_cast<const char *>(&mode));
    if (!handle)
    {
        return false;
    }

    return true;
}

inline int Stream::Close()
{
    if (handle)
    {
        return fclose((FILE *)handle);
    }

    return 0;
}

inline size_t Stream::GetSize()
{
    FILE *fp = (FILE *)handle;
    fseek(fp, 0, SEEK_END);
    auto size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    return size;
}

inline size_t Stream::Read(void *dst, size_t size, size_t count)
{
    return fread(dst, size, count, (FILE *)handle);
}

inline size_t Stream::Write(const void *src, size_t size, size_t count)
{
    return fwrite(src, size, count, (FILE *)handle);
}

inline size_t Stream::Tell() const
{
    return ftell((FILE *)handle);
}

inline int Stream::Locate(size_t pos)
{
    return fseek((FILE *)handle, pos, SEEK_SET);
}

inline int Stream::Skip(size_t offset)
{
    return fseek((FILE *)handle, offset, SEEK_CUR);
}

#endif


}
