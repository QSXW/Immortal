#include "Platform.h"

namespace Immortal
{

std::optional<std::string> FileDialogs::OpenFile(const char *filter)
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

std::optional<std::string> FileDialogs::SaveFile(const char *filter)
{
    (void)filter;
    return std::nullopt;
}

}
