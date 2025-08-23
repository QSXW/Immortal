#include "FileSystem.h"
#include <filesystem>

namespace Immortal
{

namespace FileSystem
{

namespace fs = std::filesystem;
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

void ListDirectory(const Path &_path, std::vector<DirectoryEntry> &directories, FileType filter)
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

		if (fileData.cFileName[0] != '.' &&
			fileData.cFileName[0] != '$')
		{
			DirectoryEntry entry = { (_path / fileData.cFileName).u8string(), type};
			directories.emplace_back(std::move(entry));
		}
	} while (FindNextFileW(hFind, &fileData) != 0);

	FindClose(hFind);
}

#else
void ListDirectory(const Path &_path, std::vector<DirectoryEntry> &directories, FileType filter)
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
			if (entry.GetFileName()[0] == '$' || entry.GetFileName()[0] == '.' ||
				!strcmp(entry.GetFileName(), "System Volume Information"))
			{
				continue;
			}

#ifdef _WIN32
			auto attribute = GetFileAttributesW(path.native().c_str());
			if (attribute & FILE_ATTRIBUTE_SYSTEM)
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
