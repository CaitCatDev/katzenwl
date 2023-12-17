#define _XOPEN_SOURCE 600

#include <stdlib.h>
#include <stdint.h>

#include <unistd.h>

#include <sys/shm.h>
#include <sys/mman.h>

#include <kwl/interfaces/kwl-buffer.h>
#include <kwl/renderer/allocators/allocator.h>

typedef struct kwl_shm_buffer {
	kwl_buffer_t impl;

	void *data;
	uint32_t height, width, stride;

	uint8_t bpp;
} kwl_shm_buffer_t;

void *kwl_shm_buffer_get_data_ptr(kwl_buffer_t *buffer) {
	kwl_shm_buffer_t *shm = (void *)buffer;

	return shm->data;
}

void kwl_shm_buffer_free(kwl_buffer_t *buffer) {
	kwl_shm_buffer_t *shm = (void *)buffer;

	munmap(shm->data, shm->stride * shm->height);
}

kwl_buffer_t *kwl_shm_allocate_buffer(uint32_t height, uint32_t width, uint32_t format) {
	int fd;
	kwl_shm_buffer_t *buffer = calloc(1, sizeof(kwl_shm_buffer_t));
	char template[] = "/tmp/kwl_buffer-XXXXXX";

	/* HACK: We just assume a 4 byte per pixel format
	 * this isn't a gurantee just common in modern
	 * hardware
	 */

	fd = mkstemp(template);

	unlink(template);

	ftruncate(fd, height * width * 4);


	buffer->data = mmap(NULL, height * width * 4, PROT_READ | PROT_WRITE, 
			MAP_SHARED, fd, 0);
	buffer->height = height;
	buffer->width = width;
	buffer->bpp = 32;
	buffer->stride = buffer->width * 4;
	close(fd);

	buffer->impl.caps = KWL_BUFFER_DATA_PTR_CAP;
	buffer->impl.free = kwl_shm_buffer_free;
	buffer->impl.get_data = kwl_shm_buffer_get_data_ptr;
	return &buffer->impl;
}

void kwl_shm_allocator_destory(kwl_allocator_t *allocator) {
	free(allocator);

	return;
}

kwl_allocator_t *kwl_shm_allocator_create() {
	kwl_allocator_t *shm = calloc(1, sizeof(*shm));
	
	shm->destory = kwl_shm_allocator_destory;
	shm->allocate_buffer = kwl_shm_allocate_buffer;

	return shm;
}
