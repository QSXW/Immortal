#pragma once

#include <string>
#include <optional>
#include "FileSystem/FileSystem.h"

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

    static inline char Lut[] = {
	    "3D Lookup Table(3D Lut)\0*.cube\0"
    };
};

class FileDialogs
{
public:
    static std::optional<std::string> OpenFile(const char *filter = FileFilter::None);

    static std::optional<std::string> SaveFile(const char *filter = FileFilter::None);
};

class Clipboard
{
public:
    enum class DataType
    {
        Text,
        UnicodeText
    };

public:
	static void SetData(DataType type, const void *data, size_t size);
};

class System
{
public:
	static FileSystem::Path GetTemperoryPath();

};


}
