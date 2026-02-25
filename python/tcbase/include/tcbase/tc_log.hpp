#pragma once

#include <tcbase/tc_log.h>
#include <string>
#include <cstdio>
#include <cstdarg>
#include <exception>

namespace tc {

// Static logging class for C++ code
// Usage:
//   tc::Log::info("Loading asset: %s", name.c_str());
//   tc::Log::error(e, "Failed to load asset");
//   tc::Log::warn("Value out of range: %d", value);
class Log {
public:
    // Format string logging (printf-style)
    template<typename... Args>
    static void debug(const char* format, Args... args) {
        tc_log_debug(format, args...);
    }

    template<typename... Args>
    static void info(const char* format, Args... args) {
        tc_log_info(format, args...);
    }

    template<typename... Args>
    static void warn(const char* format, Args... args) {
        tc_log_warn(format, args...);
    }

    template<typename... Args>
    static void error(const char* format, Args... args) {
        tc_log_error(format, args...);
    }

    // Simple string logging
    static void debug(const std::string& msg) {
        tc_log_debug("%s", msg.c_str());
    }

    static void info(const std::string& msg) {
        tc_log_info("%s", msg.c_str());
    }

    static void warn(const std::string& msg) {
        tc_log_warn("%s", msg.c_str());
    }

    static void error(const std::string& msg) {
        tc_log_error("%s", msg.c_str());
    }

    // Exception logging with context
    static void debug(const std::exception& e, const char* context = nullptr) {
        log_exception(TC_LOG_DEBUG, e, context);
    }

    static void info(const std::exception& e, const char* context = nullptr) {
        log_exception(TC_LOG_INFO, e, context);
    }

    static void warn(const std::exception& e, const char* context = nullptr) {
        log_exception(TC_LOG_WARN, e, context);
    }

    static void error(const std::exception& e, const char* context = nullptr) {
        log_exception(TC_LOG_ERROR, e, context);
    }

    // Exception logging with format string context
    template<typename... Args>
    static void debug(const std::exception& e, const char* format, Args... args) {
        log_exception_fmt(TC_LOG_DEBUG, e, format, args...);
    }

    template<typename... Args>
    static void info(const std::exception& e, const char* format, Args... args) {
        log_exception_fmt(TC_LOG_INFO, e, format, args...);
    }

    template<typename... Args>
    static void warn(const std::exception& e, const char* format, Args... args) {
        log_exception_fmt(TC_LOG_WARN, e, format, args...);
    }

    template<typename... Args>
    static void error(const std::exception& e, const char* format, Args... args) {
        log_exception_fmt(TC_LOG_ERROR, e, format, args...);
    }

    // Configuration
    static void set_level(tc_log_level level) {
        tc_log_set_level(level);
    }

    static void set_callback(tc_log_callback callback) {
        tc_log_set_callback(callback);
    }

private:
    static void log_exception(tc_log_level level, const std::exception& e, const char* context) {
        if (context) {
            tc_log(level, "%s: %s", context, e.what());
        } else {
            tc_log(level, "%s", e.what());
        }
    }

    template<typename... Args>
    static void log_exception_fmt(tc_log_level level, const std::exception& e, const char* format, Args... args) {
        char context[512];
        snprintf(context, sizeof(context), format, args...);
        tc_log(level, "%s: %s", context, e.what());
    }
};

} // namespace tc
