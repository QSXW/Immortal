#include "FileSystem.h"

namespace Immortal
{
namespace FileSystem
{

void ListDirectory(const Path &_path, std::vector<DirectoryEntry> &directories)
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

			directories.emplace_back(std::move(entry));
		}
	}
	catch (const std::exception &e)
	{
		LOG::ERR("Failed to list directory `{}` - {}", _path.string().c_str(), e.what());
	}
}

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
