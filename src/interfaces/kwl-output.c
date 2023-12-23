#include <stdlib.h>
#include <wayland-server.h>
#include <wayland-server-core.h>
#include <wayland-server-protocol.h>

#include <kwl/interfaces/kwl-output.h>
#include <kwl/log/logger.h>

#include <kwl-private/util/macros.h>

/* TODO: This is coded in a way where we 
 * can only have one output currently.
 * this can work but we will want to 
 * expand this to allow for more than one 
 * output.
 *
 * TODO:
 * We could make all of the wl_outputs
 * internally and export them to clients with wl_global.
 * However this is intended to be a library to help 
 * with making compositors and it would be a good idea
 * to Notify the compositor using the library with a wl_signal
 * or a generic callback or just some method that allows us to 
 * notify the server compositor "Hey there is a new output". So the 
 * compositor can handle that and chose how to draw to it or if it 
 * even wants to use it.
 *
 * TODO: Should we move global output create out of kwl_output_init?
 * As it stands right now it's technically possible for a client to
 * bind the output before the backend has initialzed it. Which could
 * Be and issue as it's possible this could cause errors. But for now
 * this works as I mainly just want to test wl events. As in theroy
 * once the client binds the output it should be given.
 *
 * TODO: Support higher global versions
 */

#define KWL_OUTPUT_VERSION 1

static const struct wl_output_interface wl_output_implementation  = {
	.release = NULL,
};

void kwl_output_bind(struct wl_client *client, void *data, uint32_t version, uint32_t id) {
	struct wl_resource *resource;
	kwl_output_t *output = data;
	kwl_log_info("%d %d\n", version, wl_output_interface.version);
	resource = wl_resource_create(client, &wl_output_interface,
			wl_output_interface.version, id);
	
	if(!resource) {
		wl_client_post_no_memory(client);
		return;
	} 
	wl_resource_set_implementation(resource,
			&wl_output_implementation, data, NULL);

	wl_output_send_geometry(resource, output->x, output->y, 
			output->pwidth, output->pheight, output->subpix, 
			output->make, output->model, output->transform);
	wl_output_send_mode(resource, output->mode.flag, output->mode.width,
			output->mode.height, output->mode.refresh);

	UNUSED(data);
	UNUSED(version);
}

kwl_output_t *kwl_output_init(struct wl_display *display) {
	kwl_output_t *output = calloc(1, sizeof(kwl_output_t));

	wl_global_create(display, &wl_output_interface, KWL_OUTPUT_VERSION, output, kwl_output_bind);

	return output;
}
