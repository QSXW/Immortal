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
    static std::optional<std::string> OpenFile(const char *filter);
    static std::optional<std::string> SaveFile(const char *filter);            
};

}
