#pragma once

#include <arch/i386/terminal.h>
#include <kernel/print.h>

namespace detail {
    enum class SeverityLevel {
        INFO,
        WARNING,
        ERROR,
    };

    struct SeverityStyle {
        const char* name;
        terminal::Color color;
    };

    constexpr SeverityStyle get_severity_style(SeverityLevel level) {
        switch (level) {
            case SeverityLevel::INFO:    return { "INF", terminal::Color::DARK_GREY };
            case SeverityLevel::WARNING: return { "WRN", terminal::Color::LIGHT_BROWN };
            case SeverityLevel::ERROR:   return { "ERR", terminal::Color::LIGHT_RED };
            default: __builtin_unreachable();
        }
    }

    template <typename... Args>
    void log(SeverityLevel severity, const char* file, int line,
        ::FormatString<Args...> format, const Args&... args)
    {
        SeverityStyle style = get_severity_style(severity);
        terminal::set_color(style.color, terminal::Color::BLACK);

        print("{} [{}:{}] ", style.name, file, line);
        println(format, args...);
        terminal::set_color(terminal::Color::LIGHT_GREY, terminal::Color::BLACK);
    }
}

#define LOG_INFO(...) detail::log(detail::SeverityLevel::INFO, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_WARN(...) detail::log(detail::SeverityLevel::WARNING, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_ERROR(...) detail::log(detail::SeverityLevel::ERROR, __FILE__, __LINE__, __VA_ARGS__)
