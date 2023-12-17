#pragma once

#include <kwl/backend/backend.h>

/**\todo implement a buffer type**/
#include <stdint.h>

typedef void kwl_buffer_t;

/**\todo Allocator generic backend. 
 * There should be a function to create a buffer
 * and a function to destroy a allocator
 * 
 * Buffers should be things we are able to
 * allocate and put on screen for example
 * The renderer should allocate a front 
 * and back buffer for the screen
 **/

typedef struct kwl_allocator kwl_allocator_t;

typedef kwl_buffer_t *(*kwl_allocate_buffer_t)(uint32_t height, uint32_t width, uint32_t format);
typedef void (*kwl_allocator_destroy_t)(kwl_allocator_t *allocator);

struct kwl_allocator {
	kwl_allocate_buffer_t allocate_buffer;
	kwl_allocator_destroy_t destory;
};


kwl_allocator_t *kwl_allocator_create(kwl_backend_t *backend);
