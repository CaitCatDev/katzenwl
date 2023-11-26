#pragma once

#include <stdint.h>
#include <wayland-server-core.h>
#include <xcb/xcb.h>
#include <xcb/xcb_errors.h>

typedef struct kwl_xcb_backend {
	xcb_connection_t *connection;
	xcb_screen_t *screen;
	xcb_window_t window;
	xcb_gcontext_t gc;

	uint32_t height, width;

	xcb_errors_context_t *err_ctx;

	/* Wayland Even source to call X event loop
	 * inside wl_display run 
	 */
	struct wl_event_source *xevent;
	struct wl_signal expose;
} kwl_xcb_backend_t;


typedef struct kwl_xcb_renderer {
	kwl_xcb_backend_t *xcb;
} kwl_xcb_renderer_t;
