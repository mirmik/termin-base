// tc_resource_map.c - Generic hashmap for resources by UUID
#include <tcbase/tc_resource_map.h>
#include <stdlib.h>
#include <string.h>

// Cross-platform strdup
#ifdef _WIN32
#define tc_strdup _strdup
#else
#define tc_strdup strdup
#endif

// ============================================================================
// Internal structures
// ============================================================================

#define ENTRY_EMPTY   0
#define ENTRY_OCCUPIED 1
#define ENTRY_DELETED  2

typedef struct {
    char* key;
    void* value;
    uint8_t state;
} resource_entry;

struct tc_resource_map {
    resource_entry* entries;
    size_t capacity;
    size_t count;
    size_t deleted;
    tc_resource_free_fn destructor;
};

// ============================================================================
// Hash function (FNV-1a)
// ============================================================================

static uint64_t hash_string(const char* str) {
    uint64_t hash = 14695981039346656037ULL;
    while (*str) {
        hash ^= (uint8_t)*str++;
        hash *= 1099511628211ULL;
    }
    return hash;
}

// ============================================================================
// Internal helpers
// ============================================================================

static void map_resize(tc_resource_map* map, size_t new_capacity);

// ============================================================================
// Lifecycle
// ============================================================================

tc_resource_map* tc_resource_map_new(tc_resource_free_fn destructor) {
    tc_resource_map* map = (tc_resource_map*)calloc(1, sizeof(tc_resource_map));
    if (!map) return NULL;

    size_t cap = 16;
    map->entries = (resource_entry*)calloc(cap, sizeof(resource_entry));
    if (!map->entries) {
        free(map);
        return NULL;
    }

    map->capacity = cap;
    map->count = 0;
    map->deleted = 0;
    map->destructor = destructor;

    return map;
}

void tc_resource_map_free(tc_resource_map* map) {
    if (!map) return;

    // Free all resources
    if (map->entries) {
        for (size_t i = 0; i < map->capacity; i++) {
            if (map->entries[i].state == ENTRY_OCCUPIED) {
                if (map->destructor && map->entries[i].value) {
                    map->destructor(map->entries[i].value);
                }
                free(map->entries[i].key);
            }
        }
        free(map->entries);
    }

    free(map);
}

void tc_resource_map_clear(tc_resource_map* map) {
    if (!map) return;

    for (size_t i = 0; i < map->capacity; i++) {
        if (map->entries[i].state == ENTRY_OCCUPIED) {
            if (map->destructor && map->entries[i].value) {
                map->destructor(map->entries[i].value);
            }
            free(map->entries[i].key);
            map->entries[i].key = NULL;
            map->entries[i].value = NULL;
            map->entries[i].state = ENTRY_EMPTY;
        } else if (map->entries[i].state == ENTRY_DELETED) {
            map->entries[i].state = ENTRY_EMPTY;
        }
    }

    map->count = 0;
    map->deleted = 0;
}

// ============================================================================
// Resize
// ============================================================================

static void map_resize(tc_resource_map* map, size_t new_capacity) {
    resource_entry* old_entries = map->entries;
    size_t old_capacity = map->capacity;

    map->entries = (resource_entry*)calloc(new_capacity, sizeof(resource_entry));
    if (!map->entries) {
        map->entries = old_entries;
        return;
    }

    map->capacity = new_capacity;
    map->count = 0;
    map->deleted = 0;

    // Reinsert all entries
    for (size_t i = 0; i < old_capacity; i++) {
        if (old_entries[i].state == ENTRY_OCCUPIED) {
            // Direct insert without destructor check
            uint64_t hash = hash_string(old_entries[i].key);
            size_t mask = map->capacity - 1;
            size_t idx = hash & mask;

            for (size_t j = 0; j < map->capacity; j++) {
                size_t probe = (idx + j) & mask;
                if (map->entries[probe].state == ENTRY_EMPTY) {
                    map->entries[probe].key = old_entries[i].key;
                    map->entries[probe].value = old_entries[i].value;
                    map->entries[probe].state = ENTRY_OCCUPIED;
                    map->count++;
                    break;
                }
            }
        }
    }

    free(old_entries);
}

// ============================================================================
// Operations
// ============================================================================

bool tc_resource_map_add(tc_resource_map* map, const char* uuid, void* resource) {
    if (!map || !uuid) return false;

    // Check if already exists
    if (tc_resource_map_contains(map, uuid)) {
        return false;
    }

    // Resize if load factor > 0.7
    if ((map->count + map->deleted) * 10 > map->capacity * 7) {
        map_resize(map, map->capacity * 2);
    }

    uint64_t hash = hash_string(uuid);
    size_t mask = map->capacity - 1;
    size_t idx = hash & mask;
    size_t first_deleted = SIZE_MAX;

    for (size_t i = 0; i < map->capacity; i++) {
        size_t probe = (idx + i) & mask;
        resource_entry* e = &map->entries[probe];

        if (e->state == ENTRY_EMPTY) {
            if (first_deleted != SIZE_MAX) {
                probe = first_deleted;
                e = &map->entries[probe];
                map->deleted--;
            }
            e->key = tc_strdup(uuid);
            e->value = resource;
            e->state = ENTRY_OCCUPIED;
            map->count++;
            return true;
        } else if (e->state == ENTRY_DELETED) {
            if (first_deleted == SIZE_MAX) first_deleted = probe;
        }
    }

    return false;
}

void* tc_resource_map_get(const tc_resource_map* map, const char* uuid) {
    if (!map || !uuid) return NULL;

    uint64_t hash = hash_string(uuid);
    size_t mask = map->capacity - 1;
    size_t idx = hash & mask;

    for (size_t i = 0; i < map->capacity; i++) {
        size_t probe = (idx + i) & mask;
        const resource_entry* e = &map->entries[probe];

        if (e->state == ENTRY_EMPTY) {
            return NULL;
        } else if (e->state == ENTRY_OCCUPIED && strcmp(e->key, uuid) == 0) {
            return e->value;
        }
    }

    return NULL;
}

bool tc_resource_map_remove(tc_resource_map* map, const char* uuid) {
    if (!map || !uuid) return false;

    uint64_t hash = hash_string(uuid);
    size_t mask = map->capacity - 1;
    size_t idx = hash & mask;

    for (size_t i = 0; i < map->capacity; i++) {
        size_t probe = (idx + i) & mask;
        resource_entry* e = &map->entries[probe];

        if (e->state == ENTRY_EMPTY) {
            return false;
        } else if (e->state == ENTRY_OCCUPIED && strcmp(e->key, uuid) == 0) {
            if (map->destructor && e->value) {
                map->destructor(e->value);
            }
            free(e->key);
            e->key = NULL;
            e->value = NULL;
            e->state = ENTRY_DELETED;
            map->count--;
            map->deleted++;
            return true;
        }
    }

    return false;
}

bool tc_resource_map_contains(const tc_resource_map* map, const char* uuid) {
    return tc_resource_map_get(map, uuid) != NULL;
}

size_t tc_resource_map_count(const tc_resource_map* map) {
    return map ? map->count : 0;
}

// ============================================================================
// Iteration
// ============================================================================

void tc_resource_map_foreach(
    tc_resource_map* map,
    tc_resource_iter_fn callback,
    void* user_data
) {
    if (!map || !callback) return;

    for (size_t i = 0; i < map->capacity; i++) {
        if (map->entries[i].state == ENTRY_OCCUPIED) {
            if (!callback(map->entries[i].key, map->entries[i].value, user_data)) {
                break;
            }
        }
    }
}
