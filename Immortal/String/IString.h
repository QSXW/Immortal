#pragma once

#include <iostream>
#include <string>
#include <format>

#include "Core.h"

namespace Immortal
{

static inline std::string Ascii2Unicode8(const std::string &str)
{
#ifdef _WIN32
    std::wstring wstr;
    wstr.resize(MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0) - 1);
    MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), wstr.data(), wstr.size());

    std::string ret;
    ret.resize(WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, NULL, NULL, NULL, NULL) - 1);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), wstr.size(), ret.data(), ret.size(), NULL, NULL);

    return ret;
#else
    return str;
#endif
}

static inline std::string Unicode82Ascii(const std::string &str)
{
#ifdef _WIN32
    std::wstring wstr;
    wstr.resize(MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0) - 1);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.size(), wstr.data(), wstr.size());

    std::string ret;
    ret.resize(WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, NULL, NULL, NULL, NULL) - 1);
    WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.size(), ret.data(), ret.size(), NULL, NULL);

    return ret;
#else
    return str;
#endif
}

static inline std::wstring Ascii2Unicode8(const std::wstring &str)
{
	assert(false && "TODO");
	return std::wstring{};
}

static inline std::wstring Unicode82Ascii(const std::wstring &str)
{
	assert(false && "TODO");
	return std::wstring{};
}

template <class T>
class BasicString
{
public:
    enum class Type
    {
        Ascii,
        Unicode8,
    };

public:
	BasicString() :
        data{},
        type{Type::Unicode8}
    {

    }

    template <class T>
    BasicString(const T &str, Type type = Type::Ascii) :
	    BasicString{}
    {
        switch (type)
        {
        case Type::Ascii:
            data = Ascii2Unicode8(str);
            break;

        case Type::Unicode8:
        default:
            data = str;
            break;
        }
    }

    size_t size() const
    {
        return data.size();
    }

    const char *c_str() const
    {
        return data.c_str();
    }

    operator T &()
    {
        return data;
    }

    operator const T &() const
    {
        return data;
    }

    bool operator<(const BasicString &other) const
    {
		return data < other.data;
    }

    Type GetType() const
    {
        return type;
    }

    std::string GetAscii() const
    {
		assert(GetType() == Type::Unicode8);
		return Unicode82Ascii(data);
    }

protected:
    T data;

    Type type;
};
 
using String  = BasicString<std::string>;

using WString = BasicString<std::wstring>;

}

template <class T>
static inline std::ostream &operator<<(std::ostream &os, const Immortal::BasicString<T> &str)
{
	return os << str.c_str();
}

template <class T, class CharT>
struct std::formatter<Immortal::BasicString<T>, CharT> : public std::formatter<const char *, CharT>
{
	template <class FC>
	auto format(const Immortal::BasicString<T> &s, FC &fc) const
	{
		return std::formatter<const char *, CharT>::format(s.c_str(), fc);
	}
};
