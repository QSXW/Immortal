#include "FileSystem.h"

namespace Immortal
{
namespace FileSystem
{

void ListDirectory(const Path &_path, std::vector<DirectoryEntry> &directories)
{
    for (auto &directory : std::filesystem::directory_iterator(_path))
    {
		FileType type = FileType::Directory;
		if (directory.is_regular_file())
		{
			type = FileType::RegularFile;
		}
		DirectoryEntry entry = { directory.path().u8string(), type };

		if (entry.GetFileName()[0] == '$' || entry.GetFileName()[0] == '.' ||
			!strcmp(entry.GetFileName(), "System Volume Information"))
		{
			continue;
		}
        directories.emplace_back(std::move(entry));
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
