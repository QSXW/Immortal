#include "impch.h"
#include "Utils/PlatformUtils.h"

#include <commdlg.h>

#include "Framework/Application.h"

namespace Immortal
{ 

std::optional<std::string> FileDialogs::OpenFile(const char *filter)
{
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

    return std::nullopt;
}

std::optional<std::string> FileDialogs::SaveFile(const char* filter)
{
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
            
    return std::nullopt;
}

}