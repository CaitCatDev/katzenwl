
#include <wayland-client.h>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>

#include <stdio.h>
#include <stdint.h>
#include <string.h>


#include <kwl/log/logger.h>
#include <kwl-private/util/macros.h>

#define MIN(x,y) x < y ? x : y

struct cstate {
	struct wl_display *display;
	struct wl_registry *registry;

	struct wl_output *output;
};

void wl_output_mode(void *data, struct wl_output *output, 
		uint32_t flags, int32_t width, int32_t height,
		int32_t refresh) {
	kwl_log_info("WL_output Mode:\n"
			"\tFlags: %u\n\tResolution: %dx%d\n"
			"\tRefresh: %d\n", flags, width, height, refresh);

}

void wl_output_geometry(void *data, struct wl_output *output,
		int32_t x, int32_t y, int32_t phywidth, int32_t phyheight,
		int32_t subpixel, const char *make, const char *model, int32_t transform) {
	
	kwl_log_info("wl_output %s:\n\tmake: %s\n"
			"\tX & Y: %d %d\n\tDimensions: %dx%d\n"
			"\tSubpixel: %d, Transform: %d\n",
			model, make, x, y, phywidth, phyheight,
			subpixel, transform);

}

void wl_output_scale(void *data, struct wl_output *output, int32_t scale) {
	printf("wl_output scale: %d\n", scale);
}

void wl_output_done(void *data, struct wl_output *output) {
	printf("wl_output Done\n");
}

static struct wl_output_listener wl_output_listener = {
	.mode = wl_output_mode,
	.geometry = wl_output_geometry,
	.scale = wl_output_scale,
	.done = wl_output_done,
};

void wl_registry_global(void *data, struct wl_registry *registry, uint32_t name, const char *interface, uint32_t version) {
	struct cstate *state;
	kwl_log_info("%d: %s(%d)\n", name, interface, version);

	state = data;

	if(strcmp(interface, wl_output_interface.name) == 0) {
		state->output = wl_registry_bind(registry, name, &wl_output_interface, MIN(2, version));
		wl_output_add_listener(state->output, &wl_output_listener, NULL);
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


	while(wl_display_dispatch(state.display)) {

	}


	UNUSED(argc);
	UNUSED(argv);
}
