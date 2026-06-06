#pragma once

#include <iostream>
#include <string>

#include "Core.h"

namespace Immortal
{

enum class StringEncoding
{
    ASCII,
    UTF8,
};

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

static inline std::string WString2String(const std::wstring &wstr, StringEncoding _encoding)
{
#ifdef _WIN32
	uint32_t encoding = _encoding == StringEncoding::UTF8 ? CP_UTF8 : CP_ACP;
	std::string str; 
	str.resize(WideCharToMultiByte(encoding, 0, wstr.c_str(), wstr.size(), NULL, 0, NULL, NULL));
	WideCharToMultiByte(encoding, 0, wstr.c_str(), wstr.size(), str.data(), str.size(), NULL, NULL);
	return str;
#else
	return str;
#endif
}

class String
{
public:
	static constexpr auto npos = std::string::npos;

public:
	String() :
        data{},
	    encoding{ StringEncoding::UTF8 }
    {

    }

    template <class T>
    requires std::is_same_v<T, std::string> || std::is_same_v<T, std::u8string>
	String(const T &str, StringEncoding encoding = StringEncoding::ASCII) :
	    String{}
    {
        if constexpr (std::is_same_v<T, std::u8string>)
        {
			encoding = StringEncoding::UTF8;
        }
		switch (encoding)
        {
        case StringEncoding::ASCII:
            if constexpr (std::is_same_v<T, std::string>)
            {
				data = Ascii2Unicode8(str);
            }
            break;

        case StringEncoding::UTF8:
        default:
			data.resize(str.size());
			memcpy(data.data(), str.data(), str.size());
            break;
        }
    }

    String(const std::string_view &view) :
        data{ view },
	    encoding{ StringEncoding::UTF8 }
    {

    }

    String(const char *str, StringEncoding encoding = StringEncoding::ASCII) :
	    String{ std::string{ str }, encoding }
    {

    }

    String(const std::wstring &str, StringEncoding encoding = StringEncoding::UTF8) :
	    data{ WString2String(str, encoding)}
    {

    }

    String(const String &other) :
	    data{ other.data },
	    encoding{ other.encoding }
    {

    }

    String(String &&other) :
	    String{}
	{
		other.Swap(*this);
	}

    String &operator=(const String &other)
	{
		String(other).Swap(*this);
		return *this;
	}

	String &operator=(String &&other)
	{
		other.Swap(*this);
		return *this;
	}

    size_t size() const
    {
        return data.size();
    }

    void resize(size_t size)
    {
		data.resize(size);
    }

    void reserve(size_t size)
    {
		data.reserve(size);
    }

    const char *c_str() const
    {
        return data.c_str();
    }

    bool empty() const
    {
		return data.empty();
    }

    operator std::string &()
    {
        return data;
    }

    operator const std::string &() const
    {
        return data;
    }

    bool operator<(const String &other) const
    {
		return data < other.data;
    }

    StringEncoding GetStringEncoding() const
    {
		return encoding;
    }

    std::string GetAscii() const
    {
		assert(GetStringEncoding() == StringEncoding::UTF8);
		return Unicode82Ascii(data);
    }

    String &operator+=(const String &other)
    {
		data += other.data;
		return *this;
    }

    operator std::u8string_view() const
    {
		return std::u8string_view{ (const char8_t *const)data.c_str(), data.size() };
    }

    size_t FindLastOf(const char c, size_t offset = 0) const
    {
		return data.find_last_of(c, offset);
    }
    
    void Swap(String &other)
    {
		data.swap(other.data);
		std::swap(encoding, other.encoding);
    }

    bool operator==(const String &other) const
    {
		return data == other.data;
    }

protected:
    std::string data;

    StringEncoding encoding;
};

}

template <>
struct std::hash<Immortal::String>
{
	std::size_t operator()(const Immortal::String &s) const noexcept
	{
		return std::hash<std::string>{}((const std::string &)s);
	}
};

static inline std::ostream &operator<<(std::ostream &os, const Immortal::String &str)
{
	return os << str.c_str();
}

#ifdef _MSC_VER
#include <format>
template <>
struct std::formatter<Immortal::String>
{
	template <class ParseContext>
	constexpr ParseContext::iterator parse(ParseContext &context)
	{
		return context.begin();
	}

	template <class FormatContext>
	FormatContext::iterator format(Immortal::String s, FormatContext &context) const
	{
		return std::format_to(context.out(), "{}", s.c_str());
	}
};

#endif
