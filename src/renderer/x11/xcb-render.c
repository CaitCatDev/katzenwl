/*Software renderer for X backend probably not going to be part of release
 * build
 */

#include <x11.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include <xcb/xcb.h>
#include <xcb/xproto.h>

void kwl_xcb_clear_screen(kwl_xcb_renderer_t *renderer, float r, float g, float b) {
	uint8_t red = 0xff * r;
	uint8_t green = 0xff * g;
	uint8_t blue = 0xff * b;
	uint32_t color = red << 16 | green << 8 | blue;
	uint32_t *data = calloc(4, renderer->xcb->width * renderer->xcb->height);

	printf("Color: %x\n", color);

	for(uint32_t y = 0; y < renderer->xcb->height; y++) {
		for(uint32_t x = 0; x < renderer->xcb->width; x++) {
			data[y * renderer->xcb->width + x] = color;
		}
	}

	xcb_put_image(renderer->xcb->connection, XCB_IMAGE_FORMAT_Z_PIXMAP, renderer->xcb->window, 
			renderer->xcb->gc, renderer->xcb->width, renderer->xcb->height, 0, 0, 0, 24, 
			4 * renderer->xcb->width * renderer->xcb->height, (void*)data);

	free(data);
	xcb_flush(renderer->xcb->connection);

}

void *kwl_renderer_init(kwl_xcb_backend_t *backend) {
	static kwl_xcb_renderer_t renderer;

	renderer.xcb = backend;
	return &renderer;
}
