#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "Types.h"
#include "Checksum.h"

#include <map>

class RF
{
public:
    RF(const std::string &filename) :
        output{ filename },
        fp{ fopen(output.c_str(), "wb+") }
    {
        if (!fp)
        {
            return;
        }
    }

    ~RF()
    {
        if (!fp)
        {
            return;
        }
        fclose(fp);
    }

    bool Writable()
    {
        return !!fp;
    }

    void EndFrame()
    {
        auto e =  Checksum::CyclicRedundancyCheck32(
            reinterpret_cast<const uint8_t *>(&std::get<1>(pair)),
            sizeof(size_t)
            );
        fwrite(&e, sizeof(uint64_t), 1, fp);
    }

    void NewFrame(uint64_t e)
    {
        pair.first  += e;
        pair.second = e;
        fwrite(&pair.first, sizeof(uint64_t), 1, fp);
    }

    void Write(const std::vector<uint8_t> &file)
    {
        NewFrame(file.size());

        fwrite(file.data(), file.size(), 1, fp);

        EndFrame();
    }

private:
    std::string output;

    std::vector<uint8_t> buffer;

    FILE *fp{ nullptr };

    std::pair<uint64_t, uint64_t> pair;
};
