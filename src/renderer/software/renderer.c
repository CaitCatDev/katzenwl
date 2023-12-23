/*Software renderer for X backend probably not going to be part of release
 * build
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include <xcb/xcb.h>
#include <xcb/xproto.h>

#include <kwl/renderer/allocators/allocator.h>
#include <kwl-private/renderer/software.h>
#include <kwl/interfaces/kwl-buffer.h>
#include <kwl/interfaces/kwl-output.h>

#include <sys/mman.h>
/*TODO: So I am starting to rewrite the output code
 * and the renderer is either going to have know which
 * output it's drawing to by:
 * A. Taking in that output every function call or
 * B. Having a bind function that makes that outputs 
 * buffer the "bound" one that all render commands will 
 * be sent to/processed on.
 *
 * HACK: For now though I am just going pass it into the function 
 * as this is **NOT** the final renderer implementation. and for a 
 * quick test this is fine
 *
 * HACK: Should each output have it's own front and back buffer 
 * as at the moment we are allocating these are render time which 
 * does cost some time. Is it worth having these pre-allocated.
 * Especially for backends like DRM that likely won't need much change.
 * As most people just run their monitor at the highest resolution supported
 * and never change it.
 */
void kwl_software_renderer_clear_screen(kwl_output_t *output, kwl_software_renderer_t *renderer, float r, float g, float b) {
	/* TODO: See about how is the most efficient way to normalize
	 * the color codes.
	 * TODO: Also maybe see about normalized pixel coords for 
	 * rendering things like polygons
	 */
	uint8_t red = 0xff * r;
	uint8_t green = 0xff * g;
	uint8_t blue = 0xff * b;
	
	uint32_t color = red << 16 | green << 8 | blue;
	uint32_t *data;
	kwl_buffer_t *buffer;
	
	kwl_allocator_t *allocator = kwl_allocator_create(renderer->backend);
	buffer = allocator->allocate_buffer(output->mode.height, output->mode.width, 0);

	data = kwl_buffer_get_data_ptr(buffer);

	printf("Color: %x\n", color);

	for(uint32_t y = 0; y < output->mode.height; y++) {
		for(uint32_t x = 0; x < output->mode.width; x++) {
			data[y * output->mode.width + x] = color;
		}
	}

	/*Commit the buffer to the output whatever that may be*/
	output->impl.commit(buffer, output);

	munmap(data, output->mode.height * output->mode.width * 4);

	buffer->free(buffer);
	allocator->destory(allocator);
}

void *kwl_software_renderer_init(kwl_backend_t *backend) {
	static kwl_software_renderer_t renderer;

	renderer.backend = backend;
	return &renderer;
}
