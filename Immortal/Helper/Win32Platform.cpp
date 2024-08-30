#include "Platform.h"

#include <Windows.h>
#include <commdlg.h>

namespace Immortal
{

std::optional<std::string> FileDialogs::OpenFile(const char *filter)
{
    static char lastDir[1024] = { 0 };

    OPENFILENAMEA ofn;
    CHAR szFile[260] = { 0 };
    ZeroMemory(&ofn, sizeof(OPENFILENAMEA));
    ofn.lStructSize = sizeof(OPENFILENAMEA);
    ofn.hwndOwner   = GetActiveWindow();;

    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    if (lastDir[0] != '\0' || GetCurrentDirectoryA(sizeof(lastDir), lastDir))
    {
        ofn.lpstrInitialDir = lastDir;
    }

    ofn.lpstrFilter = filter;
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

    if (GetOpenFileNameA(&ofn) == TRUE)
    {
        strcat(lastDir, ofn.lpstrFile);
        return ofn.lpstrFile;
    }

    return std::nullopt;
}

std::optional<std::string> FileDialogs::SaveFile(const char *filter)
{
    OPENFILENAMEA ofn;
    CHAR szFile[260] = { 0 };
    CHAR currentDir[256] = { 0 };
    ZeroMemory(&ofn, sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner   = GetActiveWindow();
    ofn.lpstrFile   = szFile;
    ofn.nMaxFile    = sizeof(szFile);
    if (GetCurrentDirectoryA(256, currentDir))
    {
        ofn.lpstrInitialDir = currentDir;
    }

    ofn.lpstrFilter = filter;
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;

    // Sets the default extension by extracting it from the filter
    ofn.lpstrDefExt = strchr(filter, '\0') + 1;

    if (GetSaveFileNameA(&ofn) == TRUE)
    {
        return ofn.lpstrFile;
    }

    return std::nullopt;
}

static uint32_t GetFormat(Clipboard::DataType type)
{
    switch (type)
    {
		case Clipboard::DataType::UnicodeText:
			return CF_UNICODETEXT;

		case Clipboard::DataType::Text:
		default:
			return CF_TEXT;
    }
}

struct ClipboardScope
{
    ClipboardScope() :
	    opened{}
    {
		opened = OpenClipboard(GetActiveWindow()) && EmptyClipboard();
    }

    ~ClipboardScope()
    {
		(void)CloseClipboard();
    }

    BOOL opened;
};

void Clipboard::SetData(DataType type, const void *data, size_t size)
{
	auto format = GetFormat(type);
    if (!IsClipboardFormatAvailable(format))
    {
		return;
    }

    ClipboardScope clipboard{};
    if (!clipboard.opened)
    {
		LOG::ERR("Failed to open clipboard!");
		return;
    }

    auto hglbCopy = GlobalAlloc(GMEM_MOVEABLE, size); 
    auto lptstrCopy = GlobalLock(hglbCopy);
	memcpy(lptstrCopy, data, size);
	GlobalUnlock(hglbCopy); 

    SetClipboardData(format, hglbCopy);

}

FileSystem::Path System::GetTemperoryPath()
{
	constexpr size_t kMaxPathLength = 1024;

	wchar_t data[kMaxPathLength];
	DWORD length = GetTempPathW(kMaxPathLength, data);
    if (!length)
    {
		return {};
    }

    return data;
}

}
