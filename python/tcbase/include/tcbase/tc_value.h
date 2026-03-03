// tc_value.h - Tagged union value type for serialization
#ifndef TC_VALUE_H
#define TC_VALUE_H

#include <stdbool.h>
#include <stddef.h>

#include <tcbase/tc_types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum tc_value_type {
    TC_VALUE_NIL = 0,
    TC_VALUE_BOOL,
    TC_VALUE_INT,
    TC_VALUE_FLOAT,
    TC_VALUE_DOUBLE,
    TC_VALUE_STRING,
    TC_VALUE_VEC3,
    TC_VALUE_QUAT,
    TC_VALUE_LIST,
    TC_VALUE_DICT,
} tc_value_type;

typedef struct tc_value tc_value;
typedef struct tc_value_list tc_value_list;
typedef struct tc_value_dict tc_value_dict;

struct tc_value_list {
    tc_value* items;
    size_t count;
    size_t capacity;
};

typedef struct tc_value_dict_entry {
    char* key;
    tc_value* value;
} tc_value_dict_entry;

struct tc_value_dict {
    tc_value_dict_entry* entries;
    size_t count;
    size_t capacity;
};

struct tc_value {
    tc_value_type type;
    union {
        bool b;
        int64_t i;
        float f;
        double d;
        char* s;
        tc_vec3 v3;
        tc_quat q;
        tc_value_list list;
        tc_value_dict dict;
    } data;
};

TC_API tc_value tc_value_nil(void);
TC_API tc_value tc_value_bool(bool v);
TC_API tc_value tc_value_int(int64_t v);
TC_API tc_value tc_value_float(float v);
TC_API tc_value tc_value_double(double v);
TC_API tc_value tc_value_string(const char* s);
TC_API tc_value tc_value_vec3(tc_vec3 v);
TC_API tc_value tc_value_quat(tc_quat q);
TC_API tc_value tc_value_list_new(void);
TC_API tc_value tc_value_dict_new(void);

TC_API void tc_value_free(tc_value* v);
TC_API tc_value tc_value_copy(const tc_value* v);
TC_API bool tc_value_equals(const tc_value* a, const tc_value* b);

TC_API void tc_value_list_push(tc_value* list, tc_value item);
TC_API tc_value* tc_value_list_get(tc_value* list, size_t index);
TC_API size_t tc_value_list_size(const tc_value* list);

TC_API void tc_value_dict_set(tc_value* dict, const char* key, tc_value item);
TC_API tc_value* tc_value_dict_get(tc_value* dict, const char* key);
TC_API bool tc_value_dict_has(const tc_value* dict, const char* key);
TC_API size_t tc_value_dict_size(const tc_value* dict);
TC_API tc_value* tc_value_dict_get_at(tc_value* dict, size_t index, const char** out_key);

#ifdef __cplusplus
}
#endif

#endif // TC_VALUE_H
