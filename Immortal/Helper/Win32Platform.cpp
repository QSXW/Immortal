#include "Platform.h"

#include <Windows.h>
#include <commdlg.h>
#include <shellapi.h>
#include <shobjidl.h>
#include <ShlObj_core.h>
#include <wrl/client.h>

namespace Immortal
{

std::optional<String> FileDialogs::OpenFile(const std::vector<COMDLG_FILTERSPEC>& filterSpecs)
{
    using Microsoft::WRL::ComPtr;
    
    ComPtr<IFileOpenDialog> pfd;
    HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
    if (SUCCEEDED(hr))
    {
        DWORD dwOptions;
        hr = pfd->GetOptions(&dwOptions);
        if (SUCCEEDED(hr))
        {
            pfd->SetOptions(dwOptions & ~FOS_ALLOWMULTISELECT);
        }

        if (!filterSpecs.empty())
        {
            hr = pfd->SetFileTypes((UINT)filterSpecs.size(), filterSpecs.data());
            if (SUCCEEDED(hr))
            {
                pfd->SetFileTypeIndex(1);
            }
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
                    String filePath{pszFilePath};
                    CoTaskMemFree(pszFilePath);
                    return filePath;
                }
            }
        }
    }

    return std::nullopt;
}

std::optional<std::vector<String>> FileDialogs::OpenMultipleFiles(const std::vector<COMDLG_FILTERSPEC>& filterSpecs)
{
    using Microsoft::WRL::ComPtr;
  
    ComPtr<IFileOpenDialog> pfd;
    HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
	if (FAILED(hr))
	{
		return std::nullopt;
	}

    DWORD dwOptions;
    hr = pfd->GetOptions(&dwOptions);
    if (SUCCEEDED(hr))
    {
        pfd->SetOptions(dwOptions | FOS_ALLOWMULTISELECT);
    }

    if (!filterSpecs.empty())
    {
        hr = pfd->SetFileTypes((UINT)filterSpecs.size(), filterSpecs.data());
        if (SUCCEEDED(hr))
        {
            pfd->SetFileTypeIndex(1);
        }
    }

    hr = pfd->Show(nullptr);
    if (SUCCEEDED(hr))
    {
        ComPtr<IShellItemArray> pResults;
        hr = pfd->GetResults(&pResults);
        if (SUCCEEDED(hr))
        {
            DWORD count = 0;
            hr = pResults->GetCount(&count);
            if (SUCCEEDED(hr) && count > 0)
            {
                std::vector<String> files;
                files.reserve(count);

                for (DWORD i = 0; i < count; i++)
                {
                    ComPtr<IShellItem> pItem;
                    hr = pResults->GetItemAt(i, &pItem);
                    if (SUCCEEDED(hr))
                    {
                        wchar_t *pszFilePath = NULL;
                        hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
                        if (SUCCEEDED(hr))
                        {
                            files.push_back(String(pszFilePath));
                            CoTaskMemFree(pszFilePath);
                        }
                    }
                }

                return files;
            }
        }
    }

    return std::nullopt;
}

std::optional<String> FileDialogs::SaveFile(const std::vector<COMDLG_FILTERSPEC>& filterSpecs)
{
    using Microsoft::WRL::ComPtr;
    
    ComPtr<IFileSaveDialog> pfd;
    HRESULT hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
    if (SUCCEEDED(hr))
    {
        DWORD dwOptions;
        hr = pfd->GetOptions(&dwOptions);
        if (SUCCEEDED(hr))
        {
            pfd->SetOptions(dwOptions | FOS_OVERWRITEPROMPT);
        }

        if (!filterSpecs.empty())
        {
            hr = pfd->SetFileTypes((UINT)filterSpecs.size(), filterSpecs.data());
            if (SUCCEEDED(hr))
            {
                pfd->SetFileTypeIndex(1);
            }
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
                    String filePath{pszFilePath};
                    CoTaskMemFree(pszFilePath);
                    return filePath;
                }
            }
        }
    }

    return std::nullopt;
}

using Microsoft::WRL::ComPtr;
std::optional<String> FileDialogs::BrowserFolder()
{
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
					return folderPath;
				}
			}
		}
	}

	return {};
}

static bool FileOperation(const std::vector<std::filesystem::path> &paths, UINT operation, const std::filesystem::path &dest = {}, bool showProgress = false)
{
	if (paths.empty())
	{
		return false;
	}

	Microsoft::WRL::ComPtr<IFileOperation> pFileOp;
	HRESULT hr = CoCreateInstance(CLSID_FileOperation, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFileOp));
	if (FAILED(hr))
	{
		return false;
	}

	pFileOp->SetOwnerWindow(GetActiveWindow());

	DWORD dwFlags = FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOF_RENAMEONCOLLISION;
	if (showProgress)
	{
		dwFlags |= FOF_SIMPLEPROGRESS | FOF_WANTNUKEWARNING;
	}
	hr = pFileOp->SetOperationFlags(dwFlags);
	if (FAILED(hr))
	{
		return false;
	}

	for (const auto& path : paths)
	{
		Microsoft::WRL::ComPtr<IShellItem> pSourceItem;
		hr = SHCreateItemFromParsingName(path.wstring().c_str(), NULL, IID_PPV_ARGS(&pSourceItem));
		if (SUCCEEDED(hr))
		{
			switch (operation)
			{
			case FO_COPY:
				if (!dest.empty())
				{
					Microsoft::WRL::ComPtr<IShellItem> pDestItem;
					hr = SHCreateItemFromParsingName(dest.wstring().c_str(), NULL, IID_PPV_ARGS(&pDestItem));
					if (SUCCEEDED(hr))
					{
						hr = pFileOp->CopyItem(pSourceItem.Get(), pDestItem.Get(), nullptr, nullptr);
					}
				}
				else
				{
					hr = pFileOp->CopyItem(pSourceItem.Get(), nullptr, nullptr, nullptr);
				}
				break;

			case FO_MOVE:
				if (!dest.empty())
				{
					Microsoft::WRL::ComPtr<IShellItem> pDestItem;
					hr = SHCreateItemFromParsingName(dest.wstring().c_str(), NULL, IID_PPV_ARGS(&pDestItem));
					if (SUCCEEDED(hr))
					{
						hr = pFileOp->MoveItem(pSourceItem.Get(), pDestItem.Get(), nullptr, nullptr);
					}
				}
				else
				{
					hr = pFileOp->MoveItem(pSourceItem.Get(), nullptr, nullptr, nullptr);
				}
				break;

			case FO_DELETE:
				hr = pFileOp->DeleteItem(pSourceItem.Get(), nullptr);
				break;
			}
		}
	}

	hr = pFileOp->PerformOperations();
	if (FAILED(hr))
	{
		return false;
	}

	BOOL aborted = FALSE;
	hr = pFileOp->GetAnyOperationsAborted(&aborted);
	return SUCCEEDED(hr) && !aborted;
}

bool FileManagement::Cut(const std::vector<std::filesystem::path> &paths)
{
	return FileOperation(paths, FO_MOVE);
}

bool FileManagement::Copy(const std::vector<std::filesystem::path> &paths)
{
	return FileOperation(paths, FO_COPY);
}

bool FileManagement::Paste(const std::filesystem::path &dest, const std::vector<std::filesystem::path> &paths)
{
	if (paths.empty())
	{
		return false;
	}
	if (!std::filesystem::is_directory(dest))
	{
		return false;
	}
	return FileOperation(paths, FO_COPY, dest);
}

bool FileManagement::MoveFileToReclycleBin(const std::vector<std::filesystem::path> &paths)
{
	return FileOperation(paths, FO_DELETE, {}, true);
}

bool FileManagement::RevealInFileExplorer(const std::filesystem::path &path)
{
	std::error_code ec;
	const std::filesystem::path target = std::filesystem::absolute(path, ec);
	const std::filesystem::path revealPath = ec ? path : target;
	if (std::filesystem::is_directory(revealPath, ec) && !ec)
	{
		return (INT_PTR)ShellExecuteW(nullptr, L"open", revealPath.c_str(), nullptr, nullptr, SW_SHOWNORMAL) > 32;
	}

	std::wstring parameters = L"/select,\"";
	parameters += revealPath.wstring();
	parameters += L"\"";
	return (INT_PTR)ShellExecuteW(nullptr, L"open", L"explorer.exe", parameters.c_str(), nullptr, SW_SHOWNORMAL) > 32;
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
		opened = OpenClipboard(GetActiveWindow());
    }

    ~ClipboardScope()
    {
		(void)CloseClipboard();
    }

	bool EraseClipBoard()
	{
		return EmptyClipboard() != 0;
	}

    BOOL opened;
};

bool Clipboard::SetData(DataType type, const void *data, size_t size)
{
    ClipboardScope clipboard{};
    if (!clipboard.opened)
    {
		LOG_ERROR("Failed to open clipboard!");
		return false;
    }

	if (!clipboard.EraseClipBoard())
	{
		return false;
	}

	auto format = GetFormat(type);
    auto hGlobalCopy = GlobalAlloc(GMEM_MOVEABLE, size);
	if (!hGlobalCopy)
	{
		LOG_ERROR("Failed to allocate memory for clipboard!");
		return false;
	}

    auto dst = GlobalLock(hGlobalCopy);
	if (!dst)
	{
		LOG_ERROR("Error when locking memory for clipboard!");
		GlobalFree(hGlobalCopy);
		return false;
	}

	memcpy(dst, data, size);
	GlobalUnlock(hGlobalCopy); 
    SetClipboardData(format, hGlobalCopy);

	return true;
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

	if (!OpenClipboard(GetActiveWindow()))
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

std::vector<std::filesystem::path> Clipboard::GetFilePaths()
{
	std::vector<std::filesystem::path> paths;

	ClipboardScope clipboard{};
	if (!clipboard.opened)
	{
		return {};
	}

	if (!IsClipboardFormatAvailable(CF_HDROP))
	{
		return {};
	}

	HDROP drop = (HDROP)GetClipboardData(CF_HDROP);
	if (!drop)
	{
		return {};
	}

	uint32_t fileCount = DragQueryFileW(drop, 0xFFFFFFFF, NULL, 0);
	if (fileCount == 0)
	{
		return {};
	}

	paths.reserve(fileCount);

	std::wstring filepath;
	for (uint32_t i = 0; i < fileCount; i++)
	{
		uint32_t length = DragQueryFileW(drop, i, NULL, 0);
		if (length == 0)
		{
			continue;
		}

		filepath.resize(length);
		if (DragQueryFileW(drop, i, &filepath[0], length + 1) == 0)
		{
			continue;
		}
		filepath.resize(length);

		paths.emplace_back(filepath);
	}

	return paths;
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
