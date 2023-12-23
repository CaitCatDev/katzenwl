#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <fcntl.h>

#include <xf86drm.h>
#include <xf86drmMode.h>

#include <kwl/log/logger.h>
#include <kwl/backend/backend.h>

#include <kwl-private/backend/drm.h>
#include <libseat.h>
/* HACK: at the moment we only have a SHM allocator
 * in the rendering code this needs to change really.
 * As it would make sense to just allocate DRM buffers 
 * directly in the render code to avoid the needless memcpy
 * of course this does currently work though but is something 
 * to address
 */
/* TODO: A system isn't guranteed to only have one GPU
 * and some users may have multiple monitors plugged 
 * into different GPUs but may want the wayland server 
 * to make use of all of these GPUs/Monitors so we need
 * to seperate seat functions out and have some way to
 * detect how many GPUs cards are present and open all 
 * of them and generate a backend for each along with 
 * this have some way to store multiple backends.
 */
static void handle_enable(struct libseat *backend, void *data) {
	(void)backend;
	int *active = (int *)data;
	(*active)++;
}

static void handle_disable(struct libseat *backend, void *data) {
	(void)backend;
	int *active = (int *)data;
	(*active)--;

	libseat_disable_seat(backend);
}

kwl_backend_t *kwl_drm_backend_init(struct wl_display *display) {
	kwl_drm_backend_t *drm;
	struct libseat *seat;
	int dev;
	int active = 0;
	struct libseat_seat_listener listener = {
		.enable_seat = handle_enable,
		.disable_seat = handle_disable,
	};
	drm = calloc(1, sizeof(kwl_drm_backend_t));

	libseat_set_log_level(LIBSEAT_LOG_LEVEL_INFO);

	seat = libseat_open_seat(&listener, &active);
	if(!seat) {
		printf("Failed to open seat: %p %m\n", seat);
		exit(1);
	}
	while(active == 0) {
		if(libseat_dispatch(seat, -1) == -1) {
			libseat_close_seat(seat);
			exit(1);
		}
	}
	kwl_log_debug("Seat active\n");

	dev = libseat_open_device(seat, "/dev/dri/card1", &drm->fd);
	kwl_log_debug("libseat open device backend: %p path: %s\nDev %d Fd %d\n",
			seat, "/dev/dri/card1", dev, drm->fd);

	drmModeResPtr res = drmModeGetResources(drm->fd);
	__builtin_dump_struct(res, &printf);
	exit(1);
	return (void*)drm;
}
