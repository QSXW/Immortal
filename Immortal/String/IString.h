#pragma once

#include <iostream>
#include <string>
#include <filesystem>

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

static inline std::wstring String2WString(const std::string &str)
{
	return std::filesystem::path(str).wstring();
}

static inline std::string WString2String(const std::wstring &wstr)
{
	return std::filesystem::path(wstr).string();
}

static inline std::u8string WString2U8String(const std::wstring &wstr)
{
	return std::filesystem::path(wstr).u8string();
}

template <class T>
static inline std::u8string String2U8String(const T &str)
{
	return std::filesystem::path(str).u8string();
}

class String
{
public:
	static constexpr auto npos = std::string::npos;

    using T    = std::u8string;
    using View = std::u8string_view;

public:
	String() :
	    _s{},
	    encoding{ StringEncoding::UTF8 }
    {

    }

    ~String()
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
	    case StringEncoding::UTF8:
			_s.resize(str.size());
			memcpy(_s.data(), str.data(), str.size());
            break;

        default:
			_s = String2U8String(str);
			break;
        }
    }

    String(const std::string_view &view, StringEncoding encoding = StringEncoding::ASCII) :
	    _s{encoding == StringEncoding::UTF8 ? (const std::u8string_view &)view : String2U8String(view)},
	    encoding{ StringEncoding::UTF8 }
    {

    }

    String(const std::u8string_view &view) :
	    _s{view},
	    encoding{ StringEncoding::UTF8 }
    {

    }

    String(const char *str, StringEncoding encoding = StringEncoding::ASCII) :
	    String{ std::string{ str }, encoding }
    {

    }

    String(const std::wstring &str) :
	    _s{WString2U8String(str)},
        encoding{ StringEncoding::UTF8 }
    {

    }

    String(const wchar_t *str) :
        String{ std::wstring{ str } }
    {

    }

    String(const std::filesystem::path &str) :
	    _s{str.u8string()},
	    encoding{ StringEncoding::UTF8 }
    {

    }

    String(const String &other) :
	    _s{other._s},
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
        return _s.size();
    }

    size_t capacity() const
    {
		return _s.capacity();
    }

    void resize(size_t size)
    {
		_s.resize(size);
    }

    void reserve(size_t size)
    {
		_s.reserve(size);
    }

    const char *c_str() const
    {
        return (const char *)_s.c_str();
    }

    char *data()
    {
		return (char *)_s.data();
    }

    bool empty() const
    {
		return _s.empty();
    }

    operator std::string &()
    {
		return (std::string &) _s;
    }

    operator const std::string &() const
    {
		return (const std::string &) _s;
    }

    bool operator<(const String &other) const
    {
		return _s < other._s;
    }

    StringEncoding GetStringEncoding() const
    {
		return encoding;
    }

    std::string GetString() const
    {
		return std::filesystem::path(_s).string();
    }

    std::wstring GetWString() const
    {
		return std::filesystem::path(_s).wstring();
    }

    const std::u8string &GetU8String() const
    {
		return _s;
    }

    std::u16string GetU16String() const
    {
		return std::filesystem::path(_s).u16string();
    }

    String &operator+=(const String &other)
    {
		_s += other._s;
		return *this;
    }

    operator std::u8string_view() const
    {
		return std::u8string_view{ (const char8_t *const)_s.c_str(), _s.size() };
    }

    constexpr size_t ReverseFind(const char c, size_t offset = std::u8string::npos) const
    {
		return _s.rfind(c, offset);
    }

    constexpr size_t Find(const String &str, size_t offset = 0) const
	{
		return _s.find(str._s, offset);
	}

    constexpr size_t Find(const char c, size_t offset = 0) const
    {
		return _s.find(c, offset);
    }

    size_t FindLastOf(const char c, size_t offset = std::u8string::npos) const
    {
		return _s.find_last_of(c, offset);
    }

    String Substring(const size_t offset, const size_t count) const
    {
		return _s.substr(offset, count);
    }

    const char &Back() const
    {
		return _s.back();
    }

    void Swap(String &other)
    {
		_s.swap(other._s);
		std::swap(encoding, other.encoding);
    }

    bool operator==(const String &other) const
    {
		return _s == other._s;
    }

    char &operator[](size_t i)
    {
		return (char &)_s[i];
    }

    std::pair<const char *, const char *> GetTuple() const
    {
		return {(const char *)_s.c_str(), (const char *)_s.c_str() + _s.size()};
    }

    std::u8string_view GetFileExtension() const
    {
		auto dot = _s.rfind('.', _s.size() - 1);
        if (dot != npos)
        {
			return std::u8string_view{_s.c_str() + dot + 1, _s.c_str() + _s.size()};
        }

	    return std::u8string_view{};
    }
   
    View GetStem() const
	{
		const T::value_type *start = _s.data();

		size_t offset = _s.size() - 1;
		auto lastDot = _s.rfind('.', offset);
		if (lastDot != std::u8string::npos)
        {
			offset = lastDot;
        }

        auto lastSlash = _s.find_last_of(u8"/\\", offset);
		if (lastSlash != T::npos)
		{
			start = &_s[lastSlash + 1];
		}

		return View{start, &_s[offset] };
	}

    friend String operator+(const String &left, const String &right);

protected:
	std::u8string _s;

    StringEncoding encoding;
};

static inline String operator+(const String &left, const String &right)
{
	String ret;
	ret._s = left._s + right._s;
	return ret;
}

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
