#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <map>

#include "Stream.h"

namespace Immortal
{

struct Chunk
{
    Chunk()
    {

    }

    Chunk(uint64_t size, const void *data) :
        size{ size },
        ptr{ reinterpret_cast<const void *>(data) }
    {

    }

    uint64_t size   = 0;
    const void *ptr = nullptr;
};

class RF
{
public:
    RF(const std::string &filename) :
        output{ filename },
        stream{ Stream::Mode::Write }
    {
        stream.Open(output);
    }

    RF(const std::string &filename, Stream::Mode mode) :
        output{ filename },
        stream{ mode }
    {
        stream.Open(output);
    }

    ~RF()
    {

    }

    bool Readable()
    {
        return stream.Readable();
    }

    bool Writable()
    {
        return stream.Writable();
    }

    void EndFrame()
    {

    }

    void NewFrame(uint64_t size)
    {
        pair.first = size;
        stream.Write<sizeof(uint64_t)>(&pair.first);
    }

    /*
     * @brief shallow copy. Must call write before release resources
     */
    template <class T>
    void Append(const std::vector<T> &data)
    {
        chunks.emplace_back(data.size() * sizeof(T), data.data());
    }

    template <class T>
    void Append(const T *constant)
    {
        chunks.emplace_back(sizeof(T), constant);
    }

    void Append(const void *data, size_t size)
    {
		chunks.emplace_back(size, data);
    }

    void Write()
    {
        for (auto &c : chunks)
        {
            NewFrame(c.size);
            stream.Write(c.ptr, c.size);
            EndFrame();
        }

        stream.Close();
    }

    template <class T>
    void Write(const std::vector<T> &file)
    {
        Append(file);
        Write();
    }

    const std::vector<Chunk> & Parse(const std::vector<uint8_t> &buf)
    {
        auto start = buf.data();
        auto ptr   = buf.data();
        auto end = ptr + buf.size();

        while (ptr < end)
        {
            Chunk chunk{};
            chunk.size = *reinterpret_cast<const uint64_t *>(ptr);
            ptr += sizeof(uint64_t);
            chunk.ptr  = ptr;

            chunks.emplace_back(std::move(chunk));
            if (chunk.size < (end - ptr))
            {
                ptr += chunk.size;
            }
            else
            {
                break;
            }
        }

        return chunks;
    }

    const std::vector<Chunk> &Read()
    {
        buffer.resize(stream.GetSize());
        stream.Read(buffer.data(), buffer.size());

        return Parse(buffer);
    }

private:
    std::string output;

    std::vector<uint8_t> buffer;

    Stream stream;

    std::pair<uint64_t, uint64_t> pair{ 0, 0 };

    std::vector<Chunk> chunks;
};
}
