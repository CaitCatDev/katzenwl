/*stdlib stuff*/
#include <stdlib.h>
#include <stdint.h>

/*XCB Stuff*/
#include <unistd.h>
#include <xcb/xcb.h>
#include <xcb/xproto.h>
#include <xcb/xcb_errors.h>

/*Wayland Stuff*/
#include <wayland-server.h>
#include <wayland-server-core.h>
#include <wayland-server-protocol.h>

/*our stuff*/
#include <kwl-private/util/macros.h>
#include <kwl-private/backend/xcb.h>

#include <kwl/interfaces/kwl-output.h>

#include <kwl/log/logger.h>

static const char *xcb_connection_err_to_str(int error) {

#define XCB_CONNECTION_ERROR(x) case XCB_ ## x : return #x

	switch(error) {
		XCB_CONNECTION_ERROR(CONN_ERROR);
		XCB_CONNECTION_ERROR(CONN_CLOSED_EXT_NOTSUPPORTED);
		XCB_CONNECTION_ERROR(CONN_CLOSED_MEM_INSUFFICIENT);
		XCB_CONNECTION_ERROR(CONN_CLOSED_REQ_LEN_EXCEED);
		XCB_CONNECTION_ERROR(CONN_CLOSED_PARSE_ERR);
		XCB_CONNECTION_ERROR(CONN_CLOSED_INVALID_SCREEN);
		default: return "Unknown";
	}
}

static void kwl_xcb_pretty_print_error(xcb_errors_context_t *ctx,
		xcb_generic_error_t *error) {
	const char *major, *minor, *extension, *error_name;

	error_name = xcb_errors_get_name_for_error(ctx, error->error_code, &extension);
	major = xcb_errors_get_name_for_major_code(ctx, error->major_code);
	minor = xcb_errors_get_name_for_minor_code(ctx, error->major_code, error->minor_code);

	kwl_log_error("xcb error: %s(%d)\n"
			"\tMajor: %s(%d)\n", error_name, error->error_code,
			major, error->major_code);

	if(error->minor_code) {
		kwl_log_printf(KWL_LOG_ERROR, "\tMinor: %s(%d)\n\tExtension: %s\n", 
				minor, error->minor_code, extension);
	} else {
		kwl_log_printf(KWL_LOG_ERROR, "\tMinor: (%d)\n", error->minor_code);
	}
}

static void kwl_xcb_expose(kwl_xcb_backend_t *xcb, xcb_generic_event_t *ev) {
	xcb_expose_event_t *expose = (void *)ev;
	xcb->height = expose->height;
	xcb->width = expose->width;

	wl_signal_emit(&xcb->impl.events.expose, NULL);
}

/* TODO: should we move the output creation as it's in a bit of akwards state.
 * AS the X server on my PC is 50/50 sometimes it sends a configure event first
 * sometimes it sends a map event first which introduces the need for configure 
 * events to check if the output member is set in xcb structure but if we where to 
 * do xcb_map_window_checked() we could check if the map is successful and then 
 * create the output then saving us having to check output is set in configure
 *
 */
static void kwl_xcb_map(kwl_xcb_backend_t *xcb, xcb_generic_event_t *ev) {
	kwl_output_t *output = kwl_output_init(xcb->display);

	xcb->output = output;

	kwl_log_warn("Map");
	
	
	output->make = "XCB Display";
	output->model = "XCB Window";
	output->mode.width = 600;
	output->mode.height = 600;
	output->mode.flag = WL_OUTPUT_MODE_CURRENT;
	output->mode.refresh = 60000;
	/*In theroy a client app should now be able to bind output and see these details*/
}

static void kwl_xcb_configure(kwl_xcb_backend_t *xcb, xcb_generic_event_t *ev) {
	xcb_configure_notify_event_t *configure = (void*)ev;
	
	kwl_log_warn("Configure %dx%d\n", configure->width, configure->height);

	if(xcb->output) {
	xcb->output->mode.height = configure->height;
	xcb->output->mode.width = configure->width;
	}
}

static int kwl_xcb_event(int fd, uint32_t mask, void *data) {
	kwl_xcb_backend_t *xcb = data;
	xcb_generic_event_t *ev;

	while((ev = xcb_poll_for_event(xcb->connection))) {
		switch(ev->response_type & 0x7f) {
			case XCB_MAP_NOTIFY:
				kwl_xcb_map(xcb, ev);
				break;
			case XCB_EXPOSE:
				kwl_xcb_expose(xcb, ev);
				break;
			case XCB_CONFIGURE_NOTIFY:
				kwl_xcb_configure(xcb, ev);
				break;
			default:
				kwl_log_warn("Unhandled Event: %s\n", xcb_errors_get_name_for_xcb_event(xcb->err_ctx, ev, NULL));
		}
		xcb_flush(xcb->connection);
		free(ev);
	}

	return 0;
}

kwl_backend_t *kwl_xcb_backend_init(struct wl_display *display) {
	int prefscreen, error;
	xcb_void_cookie_t cookie;
	kwl_xcb_backend_t *xcb;
	const xcb_setup_t *setup;
	xcb_screen_iterator_t iter;
	xcb_generic_error_t *xerror;
	uint32_t values[1], mask;
	struct wl_event_loop *loop;

	xcb = calloc(1, sizeof(*xcb));
	if(ISNULL(xcb)) {
		kwl_log_error("xcb: failed to allocate backend struct\n");
		return NULL;
	}

	xcb->connection = xcb_connect(NULL, &prefscreen);
	error = xcb_connection_has_error(xcb->connection);
	if(error) {
		kwl_log_error("xcb connection error: %s\n", 
					xcb_connection_err_to_str(error));
		free(xcb);
		return NULL;
	}

	xcb_errors_context_new(xcb->connection, &xcb->err_ctx);
		
	setup = xcb_get_setup(xcb->connection);

	iter = xcb_setup_roots_iterator(setup);
	for(; iter.rem; prefscreen--, xcb_screen_next(&iter)) {
		if(prefscreen == 0) {
			xcb->screen = iter.data;
			break;
		}
	}

	xcb->window = xcb_generate_id(xcb->connection);

	mask = XCB_CW_EVENT_MASK;
	values[0] = XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_BUTTON_PRESS |
              XCB_EVENT_MASK_BUTTON_RELEASE | XCB_EVENT_MASK_POINTER_MOTION |
              XCB_EVENT_MASK_ENTER_WINDOW | XCB_EVENT_MASK_LEAVE_WINDOW |
              XCB_EVENT_MASK_KEY_PRESS | XCB_EVENT_MASK_KEY_RELEASE |
			  XCB_EVENT_MASK_STRUCTURE_NOTIFY;
	cookie = xcb_create_window_checked(xcb->connection, 24, xcb->window, xcb->screen->root,
								0, 0, 600, 600, 1, XCB_WINDOW_CLASS_INPUT_OUTPUT, xcb->screen->root_visual,
								mask, values);

	xcb->gc = xcb_generate_id(xcb->connection);
	values[0] = xcb->screen->black_pixel;
	mask = XCB_GC_FOREGROUND;

	xcb_create_gc(xcb->connection, xcb->gc, xcb->window, mask, values);

	xerror = xcb_request_check(xcb->connection, cookie);
	if(xerror) {
		kwl_xcb_pretty_print_error(xcb->err_ctx, xerror);
		free(xerror);
	}

	xcb_map_window(xcb->connection, xcb->window);

	xcb_flush(xcb->connection);

	loop = wl_display_get_event_loop(display);
	xcb->display = display;

	wl_signal_init(&xcb->impl.events.expose);
	
	kwl_log_info("%d\n", xcb_get_file_descriptor(xcb->connection));

	xcb->xevent = wl_event_loop_add_fd(loop, xcb_get_file_descriptor(xcb->connection), 
			WL_EVENT_READABLE, kwl_xcb_event, xcb);

	xcb->height = 600;
	xcb->width = 600;

	return (kwl_backend_t *)xcb;
}
