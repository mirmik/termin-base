// tc_binding_types.h - Shared binding and handle types across termin-* libraries
#ifndef TCBASE_BINDING_TYPES_H
#define TCBASE_BINDING_TYPES_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// Language enum - which language a type is defined in
// Also used as index for bindings[] arrays
// ============================================================================

#ifndef TC_LANGUAGE_DEFINED
#define TC_LANGUAGE_DEFINED
typedef enum tc_language {
    TC_LANGUAGE_C = 0,
    TC_LANGUAGE_CXX = 1,
    TC_LANGUAGE_PYTHON = 2,
    TC_LANGUAGE_RUST = 3,
    TC_LANGUAGE_CSHARP = 4,
    TC_LANGUAGE_MAX = 8
} tc_language;
#endif

// ============================================================================
// Generic handle type + typed-handle helper macro
// ============================================================================

#ifndef TC_HANDLE_DEFINED
#define TC_HANDLE_DEFINED
typedef struct tc_handle {
    uint32_t index;
    uint32_t generation;
} tc_handle;

#ifdef __cplusplus
    #define TC_HANDLE_INVALID (tc_handle{UINT32_MAX, 0})
#else
    #define TC_HANDLE_INVALID ((tc_handle){UINT32_MAX, 0})
#endif

static inline bool tc_handle_is_invalid(tc_handle h) {
    return h.index == UINT32_MAX;
}

static inline bool tc_handle_eq(tc_handle a, tc_handle b) {
    return a.index == b.index && a.generation == b.generation;
}
#endif

#ifndef TC_DEFINE_HANDLE
#define TC_DEFINE_HANDLE(name) \
    typedef tc_handle name; \
    static inline name name##_invalid(void) { return TC_HANDLE_INVALID; } \
    static inline bool name##_is_invalid(name h) { return tc_handle_is_invalid(h); } \
    static inline bool name##_eq(name a, name b) { return tc_handle_eq(a, b); }
#endif

// ============================================================================
// Shader handle (shared typed handle)
// ============================================================================

#ifndef TC_SHADER_HANDLE_DEFINED
#define TC_SHADER_HANDLE_DEFINED
TC_DEFINE_HANDLE(tc_shader_handle)
#endif

#ifdef __cplusplus
}
#endif

#endif // TCBASE_BINDING_TYPES_H
