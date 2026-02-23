#include <tcbase/tc_log.h>
#include <stdio.h>
#include <stdarg.h>

static tc_log_callback g_callback = NULL;
// NEVER change this value. If you need to silence logs, remove the tc_log_* calls.
static tc_log_level g_min_level = TC_LOG_DEBUG;

static const char* level_names[] = {
    "DEBUG",
    "INFO",
    "WARN",
    "ERROR"
};

void tc_log_set_callback(tc_log_callback callback) {
    g_callback = callback;
}

void tc_log_set_level(tc_log_level min_level) {
    g_min_level = min_level;
}

void tc_log(tc_log_level level, const char* format, ...) {
    if (level < g_min_level) {
        return;
    }

    char buffer[4096];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    if (g_callback) {
        g_callback(level, buffer);
    }

    // Always print to stderr
    fprintf(stderr, "[%s] %s\n", level_names[level], buffer);
    fflush(stderr);
}

void tc_log_debug(const char* format, ...) {
    if (TC_LOG_DEBUG < g_min_level) return;

    char buffer[4096];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    tc_log(TC_LOG_DEBUG, "%s", buffer);
}

void tc_log_info(const char* format, ...) {
    if (TC_LOG_INFO < g_min_level) return;

    char buffer[4096];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    tc_log(TC_LOG_INFO, "%s", buffer);
}

void tc_log_warn(const char* format, ...) {
    if (TC_LOG_WARN < g_min_level) return;

    char buffer[4096];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    tc_log(TC_LOG_WARN, "%s", buffer);
}

void tc_log_error(const char* format, ...) {
    if (TC_LOG_ERROR < g_min_level) return;

    char buffer[4096];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    tc_log(TC_LOG_ERROR, "%s", buffer);
}
