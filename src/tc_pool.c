// tc_pool.c - Generic object pool implementation
#include <tcbase/tc_pool.h>
#include <tcbase/tc_log.h>
#include <string.h>

// ============================================================================
// Internal helpers
// ============================================================================

static bool pool_grow(tc_pool* pool) {
    uint32_t new_capacity = pool->capacity == 0 ? 16 : pool->capacity * 2;

    // Reallocate data array
    void* new_data = realloc(pool->data, new_capacity * pool->item_size);
    if (!new_data) {
        tc_log(TC_LOG_ERROR, "tc_pool: failed to grow data array");
        return false;
    }
    pool->data = new_data;

    // Zero-init new slots
    memset((char*)pool->data + pool->capacity * pool->item_size,
           0,
           (new_capacity - pool->capacity) * pool->item_size);

    // Reallocate generations
    uint32_t* new_gens = (uint32_t*)realloc(pool->generations, new_capacity * sizeof(uint32_t));
    if (!new_gens) {
        tc_log(TC_LOG_ERROR, "tc_pool: failed to grow generations array");
        return false;
    }
    pool->generations = new_gens;

    // Init new generations to 1
    for (uint32_t i = pool->capacity; i < new_capacity; i++) {
        pool->generations[i] = 1;
    }

    // Reallocate states
    uint8_t* new_states = (uint8_t*)realloc(pool->states, new_capacity * sizeof(uint8_t));
    if (!new_states) {
        tc_log(TC_LOG_ERROR, "tc_pool: failed to grow states array");
        return false;
    }
    pool->states = new_states;

    // Init new states to free
    memset(pool->states + pool->capacity, TC_SLOT_FREE, new_capacity - pool->capacity);

    // Reallocate free list
    uint32_t* new_free = (uint32_t*)realloc(pool->free_list, new_capacity * sizeof(uint32_t));
    if (!new_free) {
        tc_log(TC_LOG_ERROR, "tc_pool: failed to grow free list");
        return false;
    }
    pool->free_list = new_free;

    // Add new slots to free list
    for (uint32_t i = pool->capacity; i < new_capacity; i++) {
        pool->free_list[pool->free_count++] = i;
    }

    pool->capacity = new_capacity;
    return true;
}

// ============================================================================
// Lifecycle
// ============================================================================

bool tc_pool_init(tc_pool* pool, size_t item_size, uint32_t initial_capacity) {
    if (!pool || item_size == 0) return false;

    memset(pool, 0, sizeof(tc_pool));
    pool->item_size = item_size;

    if (initial_capacity > 0) {
        pool->data = calloc(initial_capacity, item_size);
        pool->generations = (uint32_t*)malloc(initial_capacity * sizeof(uint32_t));
        pool->states = (uint8_t*)calloc(initial_capacity, sizeof(uint8_t));
        pool->free_list = (uint32_t*)malloc(initial_capacity * sizeof(uint32_t));

        if (!pool->data || !pool->generations || !pool->states || !pool->free_list) {
            tc_pool_free(pool);
            return false;
        }

        // Init generations to 1
        for (uint32_t i = 0; i < initial_capacity; i++) {
            pool->generations[i] = 1;
            pool->free_list[i] = i;
        }

        pool->capacity = initial_capacity;
        pool->free_count = initial_capacity;
    }

    return true;
}

void tc_pool_free(tc_pool* pool) {
    if (!pool) return;

    free(pool->data);
    free(pool->generations);
    free(pool->states);
    free(pool->free_list);

    memset(pool, 0, sizeof(tc_pool));
}

void tc_pool_clear(tc_pool* pool) {
    if (!pool) return;

    // Bump all generations and mark as free
    for (uint32_t i = 0; i < pool->capacity; i++) {
        if (pool->states[i] == TC_SLOT_OCCUPIED) {
            pool->generations[i]++;
            pool->states[i] = TC_SLOT_FREE;
        }
    }

    // Rebuild free list
    pool->free_count = 0;
    for (uint32_t i = 0; i < pool->capacity; i++) {
        pool->free_list[pool->free_count++] = i;
    }

    pool->count = 0;
}

// ============================================================================
// Operations
// ============================================================================

tc_handle tc_pool_alloc(tc_pool* pool) {
    if (!pool) return TC_HANDLE_INVALID;

    // Grow if no free slots
    if (pool->free_count == 0) {
        if (!pool_grow(pool)) {
            return TC_HANDLE_INVALID;
        }
    }

    // Pop from free list
    uint32_t index = pool->free_list[--pool->free_count];
    pool->states[index] = TC_SLOT_OCCUPIED;
    pool->count++;

    // Zero-init the slot data
    memset((char*)pool->data + index * pool->item_size, 0, pool->item_size);

    tc_handle h;
    h.index = index;
    h.generation = pool->generations[index];
    return h;
}

bool tc_pool_free_slot(tc_pool* pool, tc_handle h) {
    if (!pool) return false;
    if (h.index >= pool->capacity) return false;
    if (pool->states[h.index] != TC_SLOT_OCCUPIED) return false;
    if (pool->generations[h.index] != h.generation) return false;

    // Mark as free
    pool->states[h.index] = TC_SLOT_FREE;
    pool->generations[h.index]++;  // Bump generation
    pool->count--;

    // Add to free list
    pool->free_list[pool->free_count++] = h.index;

    return true;
}

bool tc_pool_is_valid(const tc_pool* pool, tc_handle h) {
    if (!pool) return false;
    if (h.index >= pool->capacity) return false;
    if (pool->states[h.index] != TC_SLOT_OCCUPIED) return false;
    if (pool->generations[h.index] != h.generation) return false;
    return true;
}

void* tc_pool_get(const tc_pool* pool, tc_handle h) {
    if (!tc_pool_is_valid(pool, h)) return NULL;
    return (char*)pool->data + h.index * pool->item_size;
}

// ============================================================================
// Iteration
// ============================================================================

void tc_pool_foreach(tc_pool* pool, tc_pool_iter_fn callback, void* user_data) {
    if (!pool || !callback) return;

    for (uint32_t i = 0; i < pool->capacity; i++) {
        if (pool->states[i] == TC_SLOT_OCCUPIED) {
            void* item = (char*)pool->data + i * pool->item_size;
            if (!callback(i, item, user_data)) {
                break;
            }
        }
    }
}
