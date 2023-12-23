
/* TODO: Make backend be exported to a generic
 * set of functions allowing for more than
 * just one backend as **most** functions
 * of all backends should be a capable of
 * being exported.
 */

/*Our Headers*/
#include <kwl/backend/backend.h>
#include <kwl/log/logger.h>

#include <kwl-private/backend/xcb.h>
#include <kwl-private/backend/drm.h>

/*Wayland*/
#include <wayland-server.h>

/*C stdhdrs*/
#include <string.h>
#include <stdlib.h>

void kwl_backend_deinit(kwl_backend_t *backend) {
	backend->callbacks.deinit(backend);
}

void kwl_backend_start(kwl_backend_t *backend) {
	backend->callbacks.start(backend);
}

kwl_backend_t *kwl_backend_init_name(const char *name, struct wl_display *display) {
	
	if(strncmp(name, "xcb", 3) == 0) {
		return kwl_xcb_backend_init(display);
	} else if(strncmp(name, "drm", 3) == 0) {
		return kwl_drm_backend_init(display);
	}
	return NULL;
}

kwl_backend_t *kwl_backend_init_env(struct wl_display *display) {
	char *override = NULL;

	override = getenv("KWL_BACKEND");
	if(override) {
		return kwl_backend_init_name(override, display);
	}

	if(getenv("DISPLAY")) {
		return kwl_backend_init_name("xcb", display);
	}

	return kwl_backend_init_name("drm", display);
}
