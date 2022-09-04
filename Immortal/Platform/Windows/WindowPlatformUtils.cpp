#include "Framework/Application.h"
#include "Utils/PlatformUtils.h"

#ifdef _WIN32
#include <commdlg.h>
#endif

namespace Immortal
{

std::optional<std::string> FileDialogs::OpenFile(const char *filter)
{
#ifdef _WIN32
    static char lastDir[1024] = { 0 };

    OPENFILENAMEA ofn;
    CHAR szFile[260] = { 0 };
    ZeroMemory(&ofn, sizeof(OPENFILENAMEA));
    ofn.lStructSize = sizeof(OPENFILENAMEA);
    ofn.hwndOwner = reinterpret_cast<HWND>(Application::App()->GetWindow().Primitive());

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
#else
    char path[1024];
    FILE *f = popen("zenity --file-selection", "r");
    (void)fgets(path, 1024, f);

    int ret = pclose(f);
    if(ret< 0 )
    {
        perror("file_name_dialog()");
    }
    
    path[strlen(path) - 1] = '\0';
    return path;
#endif

    return std::nullopt;
}

std::optional<std::string> FileDialogs::SaveFile(const char *filter)
{
#ifdef _WIN32
    OPENFILENAMEA ofn;
    CHAR szFile[260] = { 0 };
    CHAR currentDir[256] = { 0 };
    ZeroMemory(&ofn, sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = reinterpret_cast<HWND>(Application::App()->GetWindow().Primitive());
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
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
#endif

    return std::nullopt;
}

}