#pragma once

#include <string>
#include <optional>
#include "FileSystem/FileSystem.h"

#ifdef _WIN32
#include <shobjidl.h> 
#endif

namespace Immortal
{

#define RAW_FILES \
    L"*.3fr;*.arq;*.arw;*.bay;*.bmq;*.cap;*.cr2;*.cr3;*.crw;*.cs1;*.dc2;*.dcr;*.dcs;*.dng;*.drf;*.erf;*.fff;*.gpr;" \
    L"*.ia;*.iiq;*.k25;*.kc2;*.kdc;*.mdc;*.mef;*.mfw;*.mos;*.mrw;*.nef;*.nrw;*.orf;*.ori;*.pef;*.pxn;*.qtk;*.raf;*.raw;" \
    L"*.rdc;*.rw1;*.rw2;*.rwl;*.rwz;*.sr2;*.srf;*.srw;*.sti;*.x3f"

class FileFilter
{
public:
	static inline const std::vector<COMDLG_FILTERSPEC> AllFiles = {
        { L"All Files", L"*.*" }
    };

    static inline const std::vector<COMDLG_FILTERSPEC> Image = {
        { L"Image Files", L"*.bmp;*.ico;*.gif;*.jpeg;*.jpg;*.png;*.tif;*.tiff;*.tga;*.hdr;*.heif;*.heic;*.hif;" RAW_FILES },
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
