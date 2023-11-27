#pragma once

#include <stdint.h>
#include <wayland-server-core.h>

#include <kwl/interfaces/kwl-output.h>

#include <xcb/xcb.h>
#include <xcb/xcb_errors.h>

typedef struct kwl_xcb_backend {
	xcb_connection_t *connection;
	xcb_screen_t *screen;
	xcb_window_t window;
	xcb_gcontext_t gc;

	uint32_t height, width;

	xcb_errors_context_t *err_ctx;

	kwl_output_t *output;

	/* Wayland Even source to call X event loop
	 * inside wl_display run 
	 */
	struct wl_display *display;
	struct wl_event_source *xevent;
	struct wl_signal expose;
} kwl_xcb_backend_t;
