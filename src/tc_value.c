#include <tcbase/tc_value.h>

#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#define tc_strdup _strdup
#else
#define tc_strdup strdup
#endif

static void list_ensure_capacity(tc_value_list* list, size_t needed) {
    if (!list || list->capacity >= needed) return;

    size_t new_cap = list->capacity == 0 ? 4 : list->capacity * 2;
    while (new_cap < needed) new_cap *= 2;

    tc_value* new_items = (tc_value*)realloc(list->items, new_cap * sizeof(tc_value));
    if (!new_items) return;

    list->items = new_items;
    list->capacity = new_cap;
}

static void dict_ensure_capacity(tc_value_dict* dict, size_t needed) {
    if (!dict || dict->capacity >= needed) return;

    size_t new_cap = dict->capacity == 0 ? 8 : dict->capacity * 2;
    while (new_cap < needed) new_cap *= 2;

    tc_value_dict_entry* new_entries =
        (tc_value_dict_entry*)realloc(dict->entries, new_cap * sizeof(tc_value_dict_entry));
    if (!new_entries) return;

    dict->entries = new_entries;
    dict->capacity = new_cap;
}

tc_value tc_value_nil(void) {
    return (tc_value){.type = TC_VALUE_NIL};
}

tc_value tc_value_bool(bool v) {
    tc_value val = {.type = TC_VALUE_BOOL};
    val.data.b = v;
    return val;
}

tc_value tc_value_int(int64_t v) {
    tc_value val = {.type = TC_VALUE_INT};
    val.data.i = v;
    return val;
}

tc_value tc_value_float(float v) {
    tc_value val = {.type = TC_VALUE_FLOAT};
    val.data.f = v;
    return val;
}

tc_value tc_value_double(double v) {
    tc_value val = {.type = TC_VALUE_DOUBLE};
    val.data.d = v;
    return val;
}

tc_value tc_value_string(const char* s) {
    tc_value val = {.type = TC_VALUE_STRING};
    val.data.s = s ? tc_strdup(s) : NULL;
    return val;
}

tc_value tc_value_vec3(tc_vec3 v) {
    tc_value val = {.type = TC_VALUE_VEC3};
    val.data.v3 = v;
    return val;
}

tc_value tc_value_quat(tc_quat q) {
    tc_value val = {.type = TC_VALUE_QUAT};
    val.data.q = q;
    return val;
}

tc_value tc_value_list_new(void) {
    tc_value val = {.type = TC_VALUE_LIST};
    val.data.list.items = NULL;
    val.data.list.count = 0;
    val.data.list.capacity = 0;
    return val;
}

tc_value tc_value_dict_new(void) {
    tc_value val = {.type = TC_VALUE_DICT};
    val.data.dict.entries = NULL;
    val.data.dict.count = 0;
    val.data.dict.capacity = 0;
    return val;
}

void tc_value_free(tc_value* v) {
    if (!v) return;

    switch (v->type) {
    case TC_VALUE_STRING:
        free(v->data.s);
        break;
    case TC_VALUE_LIST:
        for (size_t i = 0; i < v->data.list.count; i++) {
            tc_value_free(&v->data.list.items[i]);
        }
        free(v->data.list.items);
        break;
    case TC_VALUE_DICT:
        for (size_t i = 0; i < v->data.dict.count; i++) {
            free(v->data.dict.entries[i].key);
            tc_value_free(v->data.dict.entries[i].value);
            free(v->data.dict.entries[i].value);
        }
        free(v->data.dict.entries);
        break;
    default:
        break;
    }

    v->type = TC_VALUE_NIL;
    memset(&v->data, 0, sizeof(v->data));
}

tc_value tc_value_copy(const tc_value* v) {
    if (!v) return tc_value_nil();

    tc_value copy = *v;
    switch (v->type) {
    case TC_VALUE_STRING:
        copy.data.s = v->data.s ? tc_strdup(v->data.s) : NULL;
        break;
    case TC_VALUE_LIST:
        copy.data.list.items = NULL;
        copy.data.list.count = v->data.list.count;
        copy.data.list.capacity = v->data.list.count;
        if (v->data.list.count > 0) {
            copy.data.list.items = (tc_value*)malloc(v->data.list.count * sizeof(tc_value));
            for (size_t i = 0; i < v->data.list.count; i++) {
                copy.data.list.items[i] = tc_value_copy(&v->data.list.items[i]);
            }
        }
        break;
    case TC_VALUE_DICT:
        copy.data.dict.entries = NULL;
        copy.data.dict.count = v->data.dict.count;
        copy.data.dict.capacity = v->data.dict.count;
        if (v->data.dict.count > 0) {
            copy.data.dict.entries =
                (tc_value_dict_entry*)malloc(v->data.dict.count * sizeof(tc_value_dict_entry));
            for (size_t i = 0; i < v->data.dict.count; i++) {
                copy.data.dict.entries[i].key = tc_strdup(v->data.dict.entries[i].key);
                copy.data.dict.entries[i].value = (tc_value*)malloc(sizeof(tc_value));
                *copy.data.dict.entries[i].value = tc_value_copy(v->data.dict.entries[i].value);
            }
        }
        break;
    default:
        break;
    }

    return copy;
}

bool tc_value_equals(const tc_value* a, const tc_value* b) {
    if (a == b) return true;
    if (!a || !b) return false;
    if (a->type != b->type) return false;

    switch (a->type) {
    case TC_VALUE_NIL:
        return true;
    case TC_VALUE_BOOL:
        return a->data.b == b->data.b;
    case TC_VALUE_INT:
        return a->data.i == b->data.i;
    case TC_VALUE_FLOAT:
        return a->data.f == b->data.f;
    case TC_VALUE_DOUBLE:
        return a->data.d == b->data.d;
    case TC_VALUE_STRING:
        if (!a->data.s || !b->data.s) return a->data.s == b->data.s;
        return strcmp(a->data.s, b->data.s) == 0;
    case TC_VALUE_VEC3:
        return a->data.v3.x == b->data.v3.x && a->data.v3.y == b->data.v3.y &&
               a->data.v3.z == b->data.v3.z;
    case TC_VALUE_QUAT:
        return a->data.q.x == b->data.q.x && a->data.q.y == b->data.q.y &&
               a->data.q.z == b->data.q.z && a->data.q.w == b->data.q.w;
    case TC_VALUE_LIST:
        if (a->data.list.count != b->data.list.count) return false;
        for (size_t i = 0; i < a->data.list.count; i++) {
            if (!tc_value_equals(&a->data.list.items[i], &b->data.list.items[i])) return false;
        }
        return true;
    case TC_VALUE_DICT:
        if (a->data.dict.count != b->data.dict.count) return false;
        for (size_t i = 0; i < a->data.dict.count; i++) {
            const char* key = a->data.dict.entries[i].key;
            tc_value* rhs = tc_value_dict_get((tc_value*)b, key);
            if (!rhs) return false;
            if (!tc_value_equals(a->data.dict.entries[i].value, rhs)) return false;
        }
        return true;
    default:
        return false;
    }
}

void tc_value_list_push(tc_value* list, tc_value item) {
    if (!list || list->type != TC_VALUE_LIST) return;
    list_ensure_capacity(&list->data.list, list->data.list.count + 1);
    list->data.list.items[list->data.list.count++] = item;
}

tc_value* tc_value_list_get(tc_value* list, size_t index) {
    if (!list || list->type != TC_VALUE_LIST) return NULL;
    if (index >= list->data.list.count) return NULL;
    return &list->data.list.items[index];
}

size_t tc_value_list_size(const tc_value* list) {
    if (!list || list->type != TC_VALUE_LIST) return 0;
    return list->data.list.count;
}

void tc_value_dict_set(tc_value* dict, const char* key, tc_value item) {
    if (!dict || dict->type != TC_VALUE_DICT || !key) return;

    for (size_t i = 0; i < dict->data.dict.count; i++) {
        if (strcmp(dict->data.dict.entries[i].key, key) == 0) {
            tc_value_free(dict->data.dict.entries[i].value);
            *dict->data.dict.entries[i].value = item;
            return;
        }
    }

    dict_ensure_capacity(&dict->data.dict, dict->data.dict.count + 1);
    tc_value_dict_entry* e = &dict->data.dict.entries[dict->data.dict.count++];
    e->key = tc_strdup(key);
    e->value = (tc_value*)malloc(sizeof(tc_value));
    *e->value = item;
}

tc_value* tc_value_dict_get(tc_value* dict, const char* key) {
    if (!dict || dict->type != TC_VALUE_DICT || !key) return NULL;
    for (size_t i = 0; i < dict->data.dict.count; i++) {
        if (strcmp(dict->data.dict.entries[i].key, key) == 0) {
            return dict->data.dict.entries[i].value;
        }
    }
    return NULL;
}

bool tc_value_dict_has(const tc_value* dict, const char* key) {
    return tc_value_dict_get((tc_value*)dict, key) != NULL;
}

size_t tc_value_dict_size(const tc_value* dict) {
    if (!dict || dict->type != TC_VALUE_DICT) return 0;
    return dict->data.dict.count;
}

tc_value* tc_value_dict_get_at(tc_value* dict, size_t index, const char** out_key) {
    if (!dict || dict->type != TC_VALUE_DICT) return NULL;
    if (index >= dict->data.dict.count) return NULL;
    if (out_key) *out_key = dict->data.dict.entries[index].key;
    return dict->data.dict.entries[index].value;
}
