#include <kwl-private/renderer/allocators/allocator.h>
#include <stdint.h>

#define _XOPEN_SOURCE 600

#include <stdlib.h>
#include <unistd.h>

#include <sys/shm.h>
#include <sys/mman.h>

void kwl_shm_allocator_destory(kwl_allocator_t *allocator) {
	free(allocator);

	return;
}

kwl_buffer_t *kwl_shm_allocate_buffer(uint32_t height, uint32_t width, uint32_t format) {
	int fd;
	kwl_buffer_t *buffer;
	char template[] = "/tmp/kwl_buffer-XXXXXX";

	fd = mkstemp(template);

	unlink(template);

	ftruncate(fd, height * width * 4);


	buffer = mmap(NULL, height * width * 4, PROT_READ | PROT_WRITE, 
			MAP_SHARED, fd, 0);
	close(fd);

	return buffer;
}


kwl_allocator_t *kwl_shm_allocator_create() {
	kwl_allocator_t *shm = calloc(1, sizeof(*shm));
	
	shm->destory = kwl_shm_allocator_destory;
	shm->allocate_buffer = kwl_shm_allocate_buffer;

	return shm;
}
