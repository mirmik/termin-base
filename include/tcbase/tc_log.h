#ifndef TC_LOG_H
#define TC_LOG_H

// Shared logging system for all termin libraries.

#include "tcbase_api.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    TC_LOG_DEBUG = 0,
    TC_LOG_INFO = 1,
    TC_LOG_WARN = 2,
    TC_LOG_ERROR = 3
} tc_log_level;

// Callback for log interception (e.g. for editor console)
typedef void (*tc_log_callback)(tc_log_level level, const char* message);

// Set callback for log interception
TCBASE_API void tc_log_set_callback(tc_log_callback callback);

// Set minimum log level
TCBASE_API void tc_log_set_level(tc_log_level min_level);

// Log with specified level (printf-style)
TCBASE_API void tc_log(tc_log_level level, const char* format, ...);

TCBASE_API void tc_log_debug(const char* format, ...);
TCBASE_API void tc_log_info(const char* format, ...);
TCBASE_API void tc_log_warn(const char* format, ...);
TCBASE_API void tc_log_error(const char* format, ...);

#ifdef __cplusplus
}
#endif

#endif // TC_LOG_H
