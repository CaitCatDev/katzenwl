#pragma once

#include <stdint.h>
typedef struct kwl_buffer kwl_buffer_t;

void *kwl_buffer_get_data_ptr(kwl_buffer_t *buffer);
void kwl_buffer_free(kwl_buffer_t *buffer);
uint32_t kwl_buffer_get_caps(kwl_buffer_t *buffer);

#define KWL_BUFFER_DATA_PTR_CAP 0x1
#define KWL_BUFFER_SHM_FD_CAP 0x2

typedef void (*kwl_buffer_free_t)(kwl_buffer_t *buffer);
typedef void *(*kwl_buffer_get_data_t)(kwl_buffer_t *buffer);

/** Generic buffer interface
 * This will be used by kwl_*_buffer
 * structures which is just the different
 * backings for a buffer
 */
struct kwl_buffer {
	uint32_t caps;

	kwl_buffer_free_t free;
	kwl_buffer_get_data_t get_data;
};
