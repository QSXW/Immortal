#pragma once

#include <string>
#include <optional>
#include "FileSystem/FileSystem.h"

#ifdef _WIN32
#include <shobjidl.h> 
#endif

namespace Immortal
{

#define RAW_FILES L"*.cr2;*.fff;*.3fr;*.arw;*.nef;*.dng;*.raf;*.rw2;*.pef;*.srw;*.orf;*.rwz;*.bay;*.erf;*.mef;*.mos;*.mrw;*.nrw;*.raw;*.rwl;*.srw;*.x3f"

class FileFilter
{
public:
	static inline const std::vector<COMDLG_FILTERSPEC> AllFiles = {
        { L"All Files", L"*.*" }
    };

    static inline const std::vector<COMDLG_FILTERSPEC> Image = {
        { L"Image Files", L"*.bmp;*.ico;*.gif;*.jpeg;*.jpg;*.png;*.tif;*.tiff;*.tga;*.hdr;*.heif" RAW_FILES },
        { L"Raw Files", RAW_FILES }
    };

    static inline const std::vector<COMDLG_FILTERSPEC> Text = {
        { L"Text Files", L"*.txt;*.md;*.log" }
    };

    static inline const std::vector<COMDLG_FILTERSPEC> Model = {
        { L"Model Files", L"*.fbx;*.obj;*.glTF;*.blend" }
    };

    static inline const std::vector<COMDLG_FILTERSPEC> Lut = {
        { L"3D Lookup Table(3D Lut)", L"*.cube" }
    };

    static inline const std::vector<COMDLG_FILTERSPEC> Executable = {
        { L"Executable Files", L"*.exe;*.com;*.bat;*.cmd;*.msi" }
    };

    static inline const std::vector<COMDLG_FILTERSPEC> Scene = {
        { L"Immortal Scene Files", L"*.iml" }
    };
};

class FileDialogs
{
public:
    static std::optional<String> OpenFile(const std::vector<COMDLG_FILTERSPEC>& filterSpecs = {});

    static std::optional<std::vector<String>> OpenMultipleFiles(const std::vector<COMDLG_FILTERSPEC>& filterSpecs = {});

    static std::optional<String> SaveFile(const std::vector<COMDLG_FILTERSPEC>& filterSpecs = {});

    static std::optional<String> BrowserFolder();
};

class FileManagement
{
public:
	static bool Cut(const std::vector<std::filesystem::path> &paths);

	static bool Copy(const std::vector<std::filesystem::path> &paths);

    static bool Paste(const std::filesystem::path &destination, const std::vector<std::filesystem::path> &paths);

	static bool MoveFileToReclycleBin(const std::vector<std::filesystem::path> &paths);

    static bool RevealInFileExplorer(const std::filesystem::path &path);
};

class Clipboard
{
public:
    enum class DataType
    {
        Text,
        UnicodeText
    };

    enum class SetFileOperation
    {
        Copy,
        Cut
    };

public:
	static bool SetData(DataType type, const void *data, size_t size);

    static bool SetFilePaths(const std::vector<std::filesystem::path> &paths, SetFileOperation operation = SetFileOperation::Copy);

    static std::vector<std::filesystem::path> GetFilePaths();
};

class System
{
public:
	static FileSystem::Path GetTemperoryPath();
};

}
