/**
 * Copyright (C) 2021-2022, by Wu Jianhua (toqsxw@outlook.com)
 *
 * This library is distributed under the Apache-2.0 license.
 */

#include "Immortal.h"
#include "RawExtractor.h"
#include "FileSystem/FileSystem.h"

using namespace Immortal;

void ReadAllFiles(const std::string &path, std::vector<std::string> &raws)
{
    for (const auto &f : std::filesystem::directory_iterator(path))
    {
        uint64_t id = (uint64_t)FileSystem::DumpFileId(f.path().string());
        if (FileSystem::IsFormat<FileFormat::ARW>(id) ||
            FileSystem::IsFormat<FileFormat::NEF>(id) ||
            FileSystem::IsFormat<FileFormat::CR2>(id) ||
            FileSystem::IsFormat<FileFormat::CR3>(id))
        {
            raws.emplace_back(f.path().string());
        }
    }
}

int main(int argc, char **argv)
{
    LOG::Setup();

    std::vector<std::string> allFiles;
    ReadAllFiles(argv[1], allFiles);

    std::vector<uint8_t> buffer;
    Stream stream{ Stream::Mode::Read };
    stream.Open(allFiles[1]);
    stream.Read(buffer);

    Vision::CodedFrame codedFrame = { std::move(buffer) };
    Vision::RawCodec rawCodec;
    rawCodec.SetBits(Vision::RawBitDepth::_8);
    rawCodec.Decode(codedFrame);

    LOG::Release();

    return 0;
}
