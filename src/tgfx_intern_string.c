// tgfx_intern_string.c - Shared string interning implementation
#include <tcbase/tgfx_intern_string.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define TGFX_INTERN_BUCKET_COUNT 64

typedef struct tgfx_intern_entry {
    char* str;
    struct tgfx_intern_entry* next;
} tgfx_intern_entry;

static tgfx_intern_entry* g_intern_buckets[TGFX_INTERN_BUCKET_COUNT] = {0};

static uint32_t tgfx_string_hash(const char* s) {
    uint32_t hash = 5381;
    int c;
    while ((c = *s++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

const char* tgfx_intern_string(const char* s) {
    if (!s) return NULL;

    uint32_t bucket = tgfx_string_hash(s) % TGFX_INTERN_BUCKET_COUNT;

    // Search existing
    tgfx_intern_entry* entry = g_intern_buckets[bucket];
    while (entry) {
        if (strcmp(entry->str, s) == 0) {
            return entry->str;
        }
        entry = entry->next;
    }

    // Create new
    size_t len = strlen(s);
    tgfx_intern_entry* new_entry = (tgfx_intern_entry*)malloc(sizeof(tgfx_intern_entry));
    if (!new_entry) return NULL;

    new_entry->str = (char*)malloc(len + 1);
    if (!new_entry->str) {
        free(new_entry);
        return NULL;
    }

    memcpy(new_entry->str, s, len + 1);
    new_entry->next = g_intern_buckets[bucket];
    g_intern_buckets[bucket] = new_entry;

    return new_entry->str;
}

void tgfx_intern_cleanup(void) {
    for (int i = 0; i < TGFX_INTERN_BUCKET_COUNT; i++) {
        tgfx_intern_entry* entry = g_intern_buckets[i];
        while (entry) {
            tgfx_intern_entry* next = entry->next;
            free(entry->str);
            free(entry);
            entry = next;
        }
        g_intern_buckets[i] = NULL;
    }
}
