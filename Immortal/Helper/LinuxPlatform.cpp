#include "Platform.h"
#include <cstdio>
#include <cstring>

namespace Immortal
{

std::optional<String> FileDialogs::OpenFile(const char *filter)
{
    (void)filter;

    char path[1024];
    FILE *f = popen("zenity --file-selection", "r");
    (void)fgets(path, 1024, f);

    int ret = pclose(f);
    if(ret < 0)
    {
        return std::nullopt;
    }

    path[strlen(path) - 1] = '\0';

    return path;
}

std::optional<std::vector<String>> FileDialogs::OpenMultipleFiles(const char *filter)
{
    (void)filter;

    char path[1024];
    FILE *f = popen("zenity --file-selection --multiple --separator='|'", "r");
    if (!f)
    {
        return std::nullopt;
    }

    if (fgets(path, 1024, f) == nullptr)
    {
        pclose(f);
        return std::nullopt;
    }

    int ret = pclose(f);
    if (ret < 0)
    {
        return std::nullopt;
    }

    path[strlen(path) - 1] = '\0';

    std::vector<String> files;
    char *token = strtok(path, "|");
    while (token != nullptr)
    {
        files.push_back(String(token));
        token = strtok(nullptr, "|");
    }

    return files;
}

std::optional<std::string> FileDialogs::SaveFile(const char *filter)
{
    (void)filter;
    return std::nullopt;
}

}
