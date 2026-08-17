#include "FileSystem.h"
#include <chrono>
#include <filesystem>

namespace Immortal
{

namespace FileSystem
{

namespace fs = std::filesystem;

#ifdef _WIN32
static int64_t FileTimeToUnixSeconds(const FILETIME &ft)
{
	ULARGE_INTEGER uli;
	uli.LowPart = ft.dwLowDateTime;
	uli.HighPart = ft.dwHighDateTime;
	if (uli.QuadPart == 0ull)
	{
		return 0;
	}
	constexpr uint64_t WINDOWS_TICK_PER_SEC = 10000000ULL;
	constexpr uint64_t UNIX_EPOCH_DIFF_100NS = 116444736000000000ULL;
	return (int64_t)((uli.QuadPart - UNIX_EPOCH_DIFF_100NS) / WINDOWS_TICK_PER_SEC);
}
#else
static int64_t FileLastWriteToUnixSeconds(const fs::path &p)
{
	std::error_code ec;
	auto lwt = fs::last_write_time(p, ec);
	if (ec)
	{
		return 0;
	}
	auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
	    lwt - fs::file_time_type::clock::now() + std::chrono::system_clock::now());
	return (int64_t)std::chrono::duration_cast<std::chrono::seconds>(sctp.time_since_epoch()).count();
}
#endif
bool HasSubdirectory(const Path &path)
{
	try
	{
		if (!fs::exists(path) || !fs::is_directory(path))
		{
			return false;
		}

		for (const auto &entry : fs::directory_iterator(path))
		{
			if (fs::is_directory(entry.path()))
			{
				return true;
			}
		}
	}
	catch (const std::exception &e)
	{

	}

	return false;
}

#ifdef _WIN32

void ListDirectory(const Path &_path, std::vector<DirectoryEntry> &directories, FileType filter, bool includeHidden)
{
	WIN32_FIND_DATAW fileData;
	HANDLE hFind = FindFirstFileW((_path.wstring() + L"\\*").c_str(), &fileData);

	if (hFind == INVALID_HANDLE_VALUE)
	{
		LOG::ERR("Failed to find first file for directory - {} - {}", _path.string(), (int32_t) GetLastError());
		return;
	}

	do
	{
		FileType type = (fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? FileType::Directory : FileType::RegularFile;
		if (!(type & filter))
		{
			continue;
		}

		const bool navigationEntry = !wcscmp(fileData.cFileName, L".") || !wcscmp(fileData.cFileName, L"..");
		const bool protectedSystemEntry = !wcscmp(fileData.cFileName, L"System Volume Information");
		const bool hiddenEntry =
			fileData.cFileName[0] == '.' ||
			fileData.cFileName[0] == '$' ||
			(fileData.dwFileAttributes & (FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM)) != 0;
		if (!navigationEntry && !protectedSystemEntry && (includeHidden || !hiddenEntry))
		{
			DirectoryEntry entry = { (_path / fileData.cFileName).u8string(), type };
			entry.creationUnixSec   = FileTimeToUnixSeconds(fileData.ftCreationTime);
			entry.lastWriteUnixSec  = FileTimeToUnixSeconds(fileData.ftLastWriteTime);
			if (type == FileType::RegularFile)
			{
				entry.fileSize = (uint64_t(fileData.nFileSizeHigh) << 32) | uint64_t(fileData.nFileSizeLow);
			}
			directories.emplace_back(std::move(entry));
		}
	} while (FindNextFileW(hFind, &fileData) != 0);

	FindClose(hFind);
}

#else
void ListDirectory(const Path &_path, std::vector<DirectoryEntry> &directories, FileType filter, bool includeHidden)
{
	try
	{
		for (auto &directory : std::filesystem::directory_iterator(_path))
		{
			FileType type = FileType::Directory;
			if (directory.is_regular_file())
			{
				type = FileType::RegularFile;
			}

			const std::filesystem::path &path = directory.path();
			DirectoryEntry entry = { path.u8string(), type };
			entry.lastWriteUnixSec = FileLastWriteToUnixSeconds(path);
			entry.creationUnixSec  = entry.lastWriteUnixSec;
			if (type == FileType::RegularFile)
			{
				entry.fileSize = directory.file_size();
			}
			const bool protectedSystemEntry = !strcmp(entry.GetFileName(), "System Volume Information");
			const bool hiddenEntry = entry.GetFileName()[0] == '$' || entry.GetFileName()[0] == '.';
			if (protectedSystemEntry || (!includeHidden && hiddenEntry))
			{
				continue;
			}

#ifdef _WIN32
			auto attribute = GetFileAttributesW(path.native().c_str());
			if (!includeHidden && (attribute & (FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM)))
			{
				continue;
			}
#endif
			if (type == FileType::Directory)
			{
				try
				{
					entry.SetIsEmpty(std::filesystem::is_empty(path));
				}
				catch (const std::exception& e)
				{
					entry.SetIsEmpty(true);
				}
			}

			if (type & filter)
			{
				directories.emplace_back(std::move(entry));
			}
		}
	}
	catch (const std::exception &e)
	{
		LOG::ERR("Failed to list directory `{}` - {}", _path.string().c_str(), e.what());
	}
}
#endif

std::string_view ParseFileName(const String &path)
{
	size_t pos = 0;
	const auto *start = path.c_str();
	const auto *last  = start + path.size();
	const auto *p = last;
	while (p != start && !(p[-1] == '/' || p[-1] == '\\'))
	{
		--p;
	}
	
    return { p, (size_t)(last - p) };
}

void Path::GetAttribute(FileAttribute &attribute) const
{
#ifdef _WIN32
    WIN32_FILE_ATTRIBUTE_DATA data;
    if (GetFileAttributesExW(this->c_str(), GetFileExInfoStandard, &data))
    {
        attribute.size = (static_cast<uint64_t>(data.nFileSizeHigh) << 32) | data.nFileSizeLow;

        auto filetime_to_fst = [](const FILETIME &ft) -> std::filesystem::file_time_type {
            ULARGE_INTEGER ull;
            ull.LowPart = ft.dwLowDateTime;
            ull.HighPart = ft.dwHighDateTime;
            constexpr int64_t WINDOWS_TICK = 10000000LL;
            constexpr int64_t SEC_TO_UNIX_EPOCH = 11644473600LL;
            int64_t ticks = static_cast<int64_t>(ull.QuadPart) - SEC_TO_UNIX_EPOCH * WINDOWS_TICK;
            return std::filesystem::file_time_type(std::chrono::duration_cast<std::filesystem::file_time_type::duration>(std::chrono::nanoseconds(ticks * 100)));
        };
        attribute.creationTime   = filetime_to_fst(data.ftCreationTime);
        attribute.lastWriteTime  = filetime_to_fst(data.ftLastWriteTime);
        attribute.lastAccessTime = filetime_to_fst(data.ftLastAccessTime);
    }
    else
    {
        attribute.size = 0;
        attribute.creationTime = {};
        attribute.lastWriteTime = {};
        attribute.lastAccessTime = {};
    }
#else
    // 非Windows平台可用std::filesystem实现
    std::error_code ec;
    attribute.size = std::filesystem::is_regular_file(*this, ec) ? std::filesystem::file_size(*this, ec) : 0;
    attribute.creationTime = {};
    attribute.lastWriteTime = std::filesystem::last_write_time(*this, ec);
    attribute.lastAccessTime = {};
#endif
}

}
}
