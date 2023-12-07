#pragma once

namespace log {
    enum class SeverityLevel {
        INFO,
        WARNING,
        ERROR,
    };

    void log(SeverityLevel severity, const char* file, int line, const char* format, ...)
        __attribute__((format(printf, 4, 5)));
}

#define LOG_INFO(...) log::log(log::SeverityLevel::INFO, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_WARN(...) log::log(log::SeverityLevel::WARNING, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_ERROR(...) log::log(log::SeverityLevel::ERROR, __FILE__, __LINE__, __VA_ARGS__)
