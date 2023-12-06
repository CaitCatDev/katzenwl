#pragma once

#include <wayland-server.h>

/*TODO: Structure*/
typedef void kwl_seat_t;


kwl_seat_t *kwl_seat_init(struct wl_display *display);

