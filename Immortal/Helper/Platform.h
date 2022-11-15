#pragma once

#include <string>
#include <optional>

#include "Core.h"

namespace Immortal
{

class FileFilter
{
public:
    static inline char None[] = {
        "All Files\0*.*\0\0"
    };

    static inline char Scene[] = {
        "Immortal Scene\0*.iml\0"
    };

    static inline char Image[] = {
        "Image File\0*.bmp;*.ico;*.gif;*.jpeg;*.jpg;*.png;*.tif;*.tiff;*.tga;*.hdr;*.heif\0"
    };

    static inline char Model[] = {
        "Model File\0*.fbx;*.obj;*.glTF;*.blend\0"
    };
};

class FileDialogs
{
public:
    static std::optional<std::string> OpenFile(const char *filter = FileFilter::None);

    static std::optional<std::string> SaveFile(const char *filter = FileFilter::None);
};

static inline std::string Byte2UTF8(const std::string &str)
{
#ifdef _WIN32
    std::wstring wstr;
    wstr.resize(MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0));
    MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), wstr.data(), wstr.size());

    std::string ret;
    ret.resize(WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, NULL, NULL, NULL, NULL));
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), wstr.size(), ret.data(), ret.size(), NULL, NULL);

    return ret;
#else
    return str;
#endif
}

}
