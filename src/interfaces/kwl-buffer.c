#include <kwl/interfaces/kwl-buffer.h>
#include <stdint.h>

void kwl_buffer_free(kwl_buffer_t *buffer) {
	return buffer->free(buffer);
}

void *kwl_buffer_get_data_ptr(kwl_buffer_t *buffer) {
	return buffer->get_data(buffer);
}

uint32_t kwl_buffer_get_caps(kwl_buffer_t *buffer) {
	return buffer->caps;
}
