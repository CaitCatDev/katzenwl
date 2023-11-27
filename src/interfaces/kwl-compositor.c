#include <wayland-server.h>
#include <wayland-server-core.h>
#include <wayland-server-protocol.h>
#include <stdint.h>

#include <kwl/log/logger.h>
#include <kwl-private/util/macros.h>

#define WL_COMPOSITOR_VERSION 6


static void kwl_compositor_create_surface(struct wl_client *client, struct wl_resource *resource, uint32_t id) {
	UNUSED(id);
	UNUSED(resource);
	UNUSED(client);
}

static void kwl_compositor_create_region(struct wl_client *client, struct wl_resource *resource, uint32_t id) {
	UNUSED(id);
	UNUSED(resource);
	UNUSED(client);
}

static struct wl_compositor_interface compositor_implementation = {
	.create_surface = kwl_compositor_create_surface,
	.create_region = kwl_compositor_create_region,
};

static void kwl_compositor_bind(struct wl_client *client, void *data, uint32_t version, uint32_t id) {
	struct wl_resource *resource;
	kwl_log_info("%d %d\n", version, wl_compositor_interface.version);
	resource = wl_resource_create(client, &wl_compositor_interface,
			wl_compositor_interface.version, id);
	
	if(!resource) {
		wl_client_post_no_memory(client);
	} else {
		wl_resource_set_implementation(resource,
			&compositor_implementation, NULL, NULL);
	}

	UNUSED(data);
	UNUSED(version);
}

void kwl_compositor_create(struct wl_display *display) {
	wl_global_create(display, &wl_compositor_interface,
			WL_COMPOSITOR_VERSION, NULL, kwl_compositor_bind);
}
