#include <stdlib.h>
#include <wayland-server.h>
#include <wayland-server-core.h>
#include <wayland-server-protocol.h>

#include <kwl/interfaces/kwl-output.h>
#include <kwl/log/logger.h>

#include <kwl-private/util/macros.h>

#include <kwl/interfaces/kwl-seat.h>

#define KWL_SEAT_VERSION 1

static const struct wl_seat_interface wl_seat_implementation  = {
	.release = NULL,
};

/* Seat needs to be able to access backend's input devices
 * I.E. For X know that the "Input Devices" are just virtual
 * and X's Key and Pointer Events. Where as for libinput or 
 * EVDev know that the list of devices has a keyboard or has
 * A mouse
 */

static void kwl_seat_bind(struct wl_client *client, void *data, uint32_t version, uint32_t id) {
	struct wl_resource *resource;
	kwl_log_info("%d %d\n", version, wl_output_interface.version);
	resource = wl_resource_create(client, &wl_output_interface,
			wl_output_interface.version, id);
	
	if(!resource) {
		wl_client_post_no_memory(client);
		return;
	} 
	wl_resource_set_implementation(resource,
			&wl_seat_implementation, data, NULL);


	UNUSED(data);
	UNUSED(version);
}

kwl_seat_t *kwl_seat_init(struct wl_display *display) {
	kwl_seat_t *kwl_seat = calloc(1, sizeof(kwl_seat_t));

	kwl_seat->global = wl_global_create(display, &wl_seat_interface, KWL_SEAT_VERSION, NULL, kwl_seat_bind);

	return kwl_seat;
}
