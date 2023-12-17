#include <kwl/renderer/allocators/allocator.h>

kwl_allocator_t *kwl_shm_allocator_create();


kwl_allocator_t *kwl_allocator_create(kwl_backend_t *backend) {
	return kwl_shm_allocator_create();
}
