#pragma once

#include <memory>
#include <cstdio>

#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#pragma warning(pop)

#include "IObject.h"

namespace Immortal
{

class LOG
{
public:
    static void Setup(bool async = false);

    static void Release();

    static void Init(bool async = false)
    {
        Setup(async);
    }

    template <class... Args>
    static inline void WARN(spdlog::format_string_t<Args...> fmt, Args && ... args)
    {
        logger->warn(fmt, std::forward<Args>(args)...);
    }

    template <class... Args>
    static inline void INFO(spdlog::format_string_t<Args...> fmt, Args && ... args)
    {
        logger->info(fmt, std::forward<Args>(args)...);
    }

    template <class... Args>
    static inline void DEBUG(spdlog::format_string_t<Args...> fmt, Args && ... args)
    {
        logger->debug(fmt, std::forward<Args>(args)...);
    }

    template <class... Args>
    static inline void ERR(spdlog::format_string_t<Args...> fmt, Args && ... args)
    {
        logger->error(fmt, std::forward<Args>(args)...);
    }

    template <class... Args>
    static inline void FATAL(spdlog::format_string_t<Args...> fmt, Args && ... args)
    {
        logger->critical(fmt, std::forward<Args>(args)...);
    }

public:
    static std::shared_ptr<spdlog::logger> logger;
};

#define LOG_INFO(...)    LOG::INFO(__VA_ARGS__)
#define LOG_ERROR(...)   LOG::ERR(__VA_ARGS__)
#define LOG_WARNING(...) LOG::WARN(__VA_ARGS__)
#define LOG_DEBUG(...)   LOG::DEBUG(__VA_ARGS__)

template <class T>
inline const char *IClassGetName(T *_this)
requires std::derived_from<T, IClass>
{
	return InterpretAs<IClass>(_this)->GetName();
}

//template <class F, class T, class... Args>
//inline void ClogLevel(spdlog::format_string_t<Args...> fmt, T *_this, Args &&...args)
//{
//	if constexpr (std::derived_from<std::remove_pointer_t<decltype(_this), IClass>)
//	{
//		F(fmt, _this->GetName(), std::forward<Args>(args)...);
//	}
//	else
//	{
//		F(fmt, std::forward<Args>(args)...);
//	}
//}

template <class T, class... Args>
requires std::derived_from<T, IClass>
inline void ClogLevel(spdlog::format_string_t<const char *, Args...> s, T *_this, Args &&...args)
{
	LOG::INFO(s, _this->GetName(), std::forward<Args>(args)...);
}

template <class T, class... Args>
requires(!std::derived_from<T, IClass>)
inline void ClogLevel(spdlog::format_string_t<Args...> s, T *_this, Args &&...args)
{
	LOG::INFO(s, std::forward<Args>(args)...);
}

#define CLOG_LEVEL(L, S, ...)                                                       \
	if constexpr (std::derived_from<std::remove_pointer_t<decltype(this)>, IClass>) \
	{                                                                               \
		ClogLevel("[{}] " S, this, __VA_ARGS__);                                    \
	}                                                                               \
	else                                                                            \
	{                                                                               \
		ClogLevel(" " S, this, __VA_ARGS__);                                        \
	}

#define CLOG_INFO(S, ...)    CLOG_LEVEL(INFO,    S, __VA_ARGS__)
#define CLOG_ERROR(S, ...)   CLOG_LEVEL(ERR,     S, __VA_ARGS__)
#define CLOG_WARN(S, ...)    CLOG_LEVEL(WARN,    S, __VA_ARGS__)
#define CLOG_DEBUG(S, ...)   CLOG_LEVEL(DEBUG,   S, __VA_ARGS__)

}
