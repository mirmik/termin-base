// tc_resource_map.h - Generic hashmap for resources by UUID
#pragma once

#include <tcbase/tcbase_api.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// Resource map - stores void* resources by string UUID
// ============================================================================

typedef struct tc_resource_map tc_resource_map;

// Destructor callback for freeing resources
typedef void (*tc_resource_free_fn)(void* resource);

// Iterator callback: return true to continue, false to stop
typedef bool (*tc_resource_iter_fn)(const char* uuid, void* resource, void* user_data);

// ============================================================================
// Lifecycle
// ============================================================================

// Create a new resource map
// destructor is called when resources are removed or map is destroyed (can be NULL)
TCBASE_API tc_resource_map* tc_resource_map_new(tc_resource_free_fn destructor);

// Destroy map and all resources (calls destructor for each)
TCBASE_API void tc_resource_map_free(tc_resource_map* map);

// Clear all resources (calls destructor for each)
TCBASE_API void tc_resource_map_clear(tc_resource_map* map);

// ============================================================================
// Operations
// ============================================================================

// Add resource with given UUID
// Returns false if UUID already exists
TCBASE_API bool tc_resource_map_add(tc_resource_map* map, const char* uuid, void* resource);

// Get resource by UUID, returns NULL if not found
TCBASE_API void* tc_resource_map_get(const tc_resource_map* map, const char* uuid);

// Remove resource by UUID (calls destructor)
// Returns true if removed
TCBASE_API bool tc_resource_map_remove(tc_resource_map* map, const char* uuid);

// Check if UUID exists
TCBASE_API bool tc_resource_map_contains(const tc_resource_map* map, const char* uuid);

// Get number of resources
TCBASE_API size_t tc_resource_map_count(const tc_resource_map* map);

// ============================================================================
// Iteration
// ============================================================================

// Iterate over all resources
// Callback receives uuid, resource pointer, and user_data
// Return false from callback to stop iteration
TCBASE_API void tc_resource_map_foreach(
    tc_resource_map* map,
    tc_resource_iter_fn callback,
    void* user_data
);

#ifdef __cplusplus
}
#endif
