#pragma once

#include <memory>
#include <cstdio>

#pragma warning(push, 0)
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#pragma warning(pop)

namespace Immortal
{

class LOG
{
public:
    static void INIT();

    template <class... Args>
    static inline constexpr void WARN(Args&& ... args)
    {
        logger->warn(std::forward<Args>(args)...);
    }

    template <class... Args>
    static inline constexpr void INFO(Args&& ... args)
    {
        logger->info(std::forward<Args>(args)...);
    }

    template <class... Args>
    static inline constexpr void DEBUG(Args&& ... args)
    {
        logger->debug(std::forward<Args>(args)...);
    }

    template <class... Args>
    static inline constexpr void ERR(Args&& ... args)
    {
        logger->error(std::forward<Args>(args)...);
    }

    template <class... Args>
    static inline constexpr void FATAL(Args&& ... args)
    {
        logger->critical(std::forward<Args>(args)...);
    }

private:
    static std::shared_ptr<spdlog::logger> logger;
};

}
