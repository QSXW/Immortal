#pragma once

#include <memory>
#include <cstdio>

#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#pragma warning(pop)

#include <time.h>

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

    template <bool On = true, class... Args>
    static inline void WARN(spdlog::format_string_t<Args...> fmt, Args && ... args)
    {
        if constexpr (On)
        {
            logger->warn(fmt, std::forward<Args>(args)...);
        }
    }

    template <class... Args>
    static inline void INFO(spdlog::format_string_t<Args...> fmt, Args && ... args)
    {
        logger->info(fmt, std::forward<Args>(args)...);
    }

    template <bool On = true, class... Args>
    static inline void DEBUG(spdlog::format_string_t<Args...> fmt, Args && ... args)
    {
        if constexpr (On)
        {
            logger->debug(fmt, std::forward<Args>(args)...);
        }
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

private:
    static std::shared_ptr<spdlog::logger> logger;
};

struct Profiler
{

Profiler(const char *msg = "") :
    start{ clock() },
    end{ 0 }
{
    LOG::INFO("-> {0}", msg);
}

~Profiler()
{
    end = clock();
    double duration = ((double)end - (double)start) / CLOCKS_PER_SEC;
    LOG::INFO("<- {0} (s)", duration);
}

time_t start;
time_t end;

};

struct ErrorHandle
{
public:
    void Upload(const std::string &s)
    {
        buffer.append(s);
    }

    const std::string &Retrieve() const
    {
        return buffer;
    }

    const char *Raw()
    {
        return buffer.c_str();
    }

private:
    std::string buffer;
};

}
