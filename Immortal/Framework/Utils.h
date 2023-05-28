#pragma once

#include <locale>
#include <codecvt>

namespace Immortal
{

static inline std::wstring ToWString(const std::string &str)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
    return converter.from_bytes(str);
}

static inline std::string ToString(const std::wstring &wstr)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
    return converter.to_bytes(wstr);
}

}
