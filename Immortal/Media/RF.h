#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <map>

#include "Types.h"
#include "Checksum.h"
#include "Stream.h"

using namespace sl;

class RF
{
public:
    RF(const std::string &filename) :
        output{ filename },
        stream{ output, Stream::Mode::Write }
    {
        if (!stream.Readable())
        {
            return;
        }
    }

    ~RF()
    {

    }

    bool Writable()
    {
        return stream.Writable();
    }

    void EndFrame()
    {
        auto e =  Checksum::CyclicRedundancyCheck32(
            reinterpret_cast<const uint8_t *>(&std::get<1>(pair)),
            sizeof(size_t)
            );
        stream.Write<sizeof(uint64_t)>(&e);
    }

    void NewFrame(uint64_t e)
    {
        pair.first  += e;
        pair.second = e;
        stream.Write<sizeof(uint64_t)>(&pair.first);
    }

    void Write(const std::vector<uint8_t> &file)
    {
        NewFrame(file.size());

        stream.Write(file.data(), file.size());

        EndFrame();
    }

private:
    std::string output;

    std::vector<uint8_t> buffer;

    Stream stream;

    std::pair<uint64_t, uint64_t> pair;
};
