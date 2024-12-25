#include "Platform.h"

#include <Windows.h>
#include <commdlg.h>
#include <shellapi.h>
#include <shobjidl.h>
#include <ShlObj_core.h>
#include <wrl/client.h>

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

using Microsoft::WRL::ComPtr;
String FileDialogs::BrowserFolder()
{
	CoInitialize(NULL);

	ComPtr<IFileDialog> pfd;
	HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
	if (SUCCEEDED(hr))
	{
		DWORD dwOptions;
		hr = pfd->GetOptions(&dwOptions);
		if (SUCCEEDED(hr))
		{
			pfd->SetOptions(dwOptions | FOS_PICKFOLDERS);
		}

        hr = pfd->Show(nullptr);
		if (SUCCEEDED(hr))
		{
			ComPtr<IShellItem> pResult;
			hr = pfd->GetResult(&pResult);
			if (SUCCEEDED(hr))
			{
				wchar_t *pszFilePath = NULL;
				hr = pResult->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
				if (SUCCEEDED(hr))
				{
					String folderPath(pszFilePath);
					CoTaskMemFree(pszFilePath);
					CoUninitialize();
					return folderPath;
				}
			}
		}
	}

	CoUninitialize();
	return {};
}

static bool FileOperation(const std::vector<std::filesystem::path> &paths, UINT operation)
{
	std::wstring pathsWithDoubleNull;
	for (auto &path : paths)
	{
		pathsWithDoubleNull += path.wstring() + L'\0';
	}

	pathsWithDoubleNull += L'\0';
	SHFILEOPSTRUCTW fileOp = {0};
	fileOp.wFunc  = operation;
	fileOp.pFrom  = pathsWithDoubleNull.c_str();
	fileOp.fFlags = FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOF_SILENT;
	int result = SHFileOperationW(&fileOp);
	return (result == 0);
}

bool FileManagement::Cut(const std::vector<std::filesystem::path> &paths)
{
	return FileOperation(paths, FO_MOVE);
}

bool FileManagement::Copy(const std::vector<std::filesystem::path> &paths)
{
	return FileOperation(paths, FO_COPY);
}

bool FileManagement::MoveFileToReclycleBin(const std::vector<std::filesystem::path> &paths)
{
	return FileOperation(paths, FO_DELETE);
}

bool FileManagement::RevealInFileExplorer(const std::filesystem::path &path)
{
	system(("explorer.exe /select, " + path.string()).c_str());
	return true;
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

bool Clipboard::SetFilePaths(const std::vector<std::filesystem::path> &paths, SetFileOperation operation)
{
	size_t totalSize = sizeof(DROPFILES) + sizeof(wchar_t); 
	for (const auto &path : paths)
	{
		totalSize += (path.native().size() + 1) * sizeof(path.native()[0]);        // +1 for null terminator
	}

	HGLOBAL hGlobal = GlobalAlloc(GHND, totalSize);
	if (!hGlobal)
	{
		LOG::ERR("GlobalAlloc failed.");
		return false;
	}

	DROPFILES *pDropFiles = static_cast<DROPFILES *>(GlobalLock(hGlobal));
	if (!pDropFiles)
	{
		GlobalFree(hGlobal);
		LOG::ERR("GlobalLock failed.");
		return false;
	}

	pDropFiles->pFiles = sizeof(DROPFILES);
	pDropFiles->pt.x   = 0;
	pDropFiles->pt.y   = 0;
	pDropFiles->fNC    = FALSE;
	pDropFiles->fWide  = TRUE;

	wchar_t *pDst = reinterpret_cast<wchar_t *>(pDropFiles + 1);
	for (const auto &path : paths)
	{
		size_t size = path.native().size() + 1;
		wcscpy_s(pDst, size, path.native().c_str());
		pDst += size;
	}
	*pDst = L'\0';

	GlobalUnlock(hGlobal);

	if (!OpenClipboard(NULL))
	{
		GlobalFree(hGlobal);
		LOG::ERR("OpenClipboard failed.");
		return false;
	}

	EmptyClipboard();
	if (!SetClipboardData(CF_HDROP, hGlobal))
	{
		GlobalFree(hGlobal);
		CloseClipboard();
		LOG::ERR("SetClipboardData failed.");
		return false;
	}

	HANDLE hGlobalEffect = GlobalAlloc(GHND, sizeof(DWORD));
	if (hGlobalEffect)
	{
		DWORD *pEffect = static_cast<DWORD *>(GlobalLock(hGlobalEffect));
		if (pEffect)
		{
			*pEffect = operation == SetFileOperation::Cut ? DROPEFFECT_MOVE : DROPEFFECT_COPY;
			GlobalUnlock(hGlobalEffect);
			SetClipboardData(RegisterClipboardFormat(CFSTR_PREFERREDDROPEFFECT), hGlobalEffect);
		}
	}

	CloseClipboard();
	return true;
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
