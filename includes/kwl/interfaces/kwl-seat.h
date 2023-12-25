#pragma once

#include <wayland-server.h>

/*TODO: Structure*/
typedef struct {
	struct wl_global *global;
} kwl_seat_t;


kwl_seat_t *kwl_seat_init(struct wl_display *display);
