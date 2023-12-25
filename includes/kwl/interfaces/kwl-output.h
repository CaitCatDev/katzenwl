#pragma once

#include <stdint.h>
#include <wayland-server.h>
#include <wayland-server-protocol.h>
#include <wayland-server-core.h>
#include <xcb/xproto.h>

#include <kwl/interfaces/kwl-buffer.h>
#include <kwl/backend/backend.h>


typedef struct {
	enum wl_output_mode flag;
	int32_t width, height;
	int32_t refresh;
} kwl_output_mode_t;

/*callback frame function for kwl_output_t*/

struct kwl_output_events {
	struct wl_signal frame;
};

typedef struct kwl_output kwl_output_t;


/* Implementation functions to call on certain
 * output requests like flushing an image to 
 * the screen. Basically these are function pointers
 * to backend code that handles windows/display surfaces 
 * for that backend.
 */
typedef void (*kwl_output_commit_buffer_t)(kwl_buffer_t *buffer, kwl_output_t *output);
struct kwl_output_implementation {
	kwl_output_commit_buffer_t commit;
};

struct kwl_output {
	kwl_backend_t *backend;

	struct wl_global *global;

	char *make;
	char *model;

	int32_t pwidth, pheight;
	int32_t x, y;

	int32_t subpix, transform; /*TODO: ENUMS*/

	char *name;
		
	kwl_output_mode_t mode;
	float color; /*HACK: Here for a test*/

	/* Realistically each output should have it's own set of buffers
	 * a front and a back buffer as of right now output is very much
	 * a display backend thing and buffers are part of the rendering 
	 * infrastructure so we need to intergrate these two things 
	 * on something like DRM they should be a pair of DRM-Dumb/GBM 
	 * buffers but on X/wayland they could really be any sort of buffer
	 */
	struct kwl_output_implementation impl;
	struct kwl_output_events events;

	struct wl_list link;
};

void kwl_output_bind(struct wl_client *client, void *data, uint32_t version, uint32_t id);
kwl_output_t *kwl_output_init(struct wl_display *display);	
