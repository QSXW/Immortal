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

		if (fileData.cFileName[0] != '.')
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
	const char *start = path.c_str();
	const char *last  = start + path.size();
	const char *p = last;
	while (p != start && !(p[-1] == '/' || p[-1] == '\\'))
	{
		--p;
	}
	
    return { p, (size_t)(last - p) };
}

}
}
