
#include <wayland-client.h>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>

#include <stdio.h>
#include <stdint.h>
#include <string.h>


#include <kwl/log/logger.h>
#include <kwl-private/util/macros.h>

struct cstate {
	struct wl_display *display;
	struct wl_registry *registry;
	struct wl_compositor *compositor;
};

void wl_registry_global(void *data, struct wl_registry *registry, uint32_t name, const char *interface, uint32_t version) {
	struct cstate *state;
	kwl_log_info("%d: %s(%d)\n", name, interface, version);

	state = data;

	if(strcmp(interface, wl_compositor_interface.name) == 0) {
		state->compositor = wl_registry_bind(registry, name, &wl_compositor_interface, version);
	}
}

void wl_registry_global_remove(void *data, struct wl_registry *registry, uint32_t name) {
	UNUSED(data);
	UNUSED(registry);
	UNUSED(name);
}

static const struct wl_registry_listener wl_registry_listener = {
	.global = wl_registry_global,
	.global_remove = wl_registry_global_remove,
};

int main(int argc, char **argv) {
	struct cstate state;

	state.display = wl_display_connect(NULL);

	state.registry = wl_display_get_registry(state.display);
	wl_registry_add_listener(state.registry, &wl_registry_listener, &state);
	
	wl_display_roundtrip(state.display);

	wl_display_flush(state.display);

	while(wl_display_prepare_read(state.display) != 0) {
		wl_display_dispatch_pending(state.display);
	}

	wl_display_read_events(state.display);


	UNUSED(argc);
	UNUSED(argv);
}
