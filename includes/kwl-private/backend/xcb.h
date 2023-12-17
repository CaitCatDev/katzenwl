#pragma once

#include <stdint.h>
#include <wayland-server-core.h>

#include <kwl/interfaces/kwl-output.h>
#include <kwl/backend/backend.h>

#include <xcb/xcb.h>
#include <xcb/xcb_errors.h>

/** 
 * \file xcb.h
 * \author Caitcat
 * \date 6 Dec 2023
 *
 * Generic xcb backend functions internal structures and 
 * typedefs this is not intended for use for external 
 * apps using this lib.
 *
 * \internal
 */

typedef struct kwl_xcb_backend {
	kwl_backend_t impl;

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
} kwl_xcb_backend_t;

kwl_backend_t *kwl_xcb_backend_init(struct wl_display *display);
