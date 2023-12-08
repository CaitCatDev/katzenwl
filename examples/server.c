#include <wayland-server.h>
#include <wayland-server-core.h>
#include <wayland-server-protocol.h>

#include <kwl/log/logger.h>
#include <kwl/interfaces/kwl-seat.h>

#include <kwl/backend/backend.h>

#include <kwl-private/util/macros.h>
#include <kwl-private/renderer/xcb.h>

#include <unistd.h>
#include <signal.h>
#include <wayland-util.h>


void kwl_compositor_create(struct wl_display *display);	
int stdin_keypress(int fd, unsigned int mask, void *data) {
	char ch;
	
	read(fd, &ch, 1);

	if(ch == 'q') {	
		wl_display_terminate(data);
	}
	UNUSED(mask);

	return 0;
}

int sigint_handler(int signo, void *data) {
	kwl_log_info("Sigint Recieved\n");
	wl_display_terminate(data);

	return 0;
	UNUSED(signo);
}



void kwl_compositor_create(struct wl_display *display);

void kwl_xcb_clear_screen(kwl_xcb_renderer_t *renderer, float r, float g, float b);
void *kwl_renderer_init(kwl_xcb_backend_t *backend);

struct kwl_server {
	struct wl_display *display;
	kwl_xcb_renderer_t *renderer;
	struct wl_listener listener;
};

void kwl_expose_notify(struct wl_listener *listen, void *data) {
	struct kwl_server *srv = wl_container_of(listen, srv, listener);


	kwl_xcb_clear_screen(srv->renderer, 0.2f, 0.2f, 0.3f);
}

int main(int argc, char *argv[]) {
	struct kwl_server srv;
	struct wl_event_loop *loop;
	struct wl_event_source *key, *sigint;
	const char *socket;

	srv.display = wl_display_create();
	
	socket = wl_display_add_socket_auto(srv.display);

	kwl_log_info("%s\n", socket);

	loop = wl_display_get_event_loop(srv.display);

	kwl_compositor_create(srv.display);
	wl_display_init_shm(srv.display);

	key = wl_event_loop_add_fd(loop, STDIN_FILENO, WL_EVENT_READABLE, stdin_keypress, srv.display);
	sigint = wl_event_loop_add_signal(loop, SIGINT, sigint_handler, srv.display);
	kwl_backend_t *backend = kwl_xcb_backend_init(srv.display);
	srv.renderer = kwl_renderer_init((void *)backend);
	

	kwl_seat_init(srv.display);

	srv.listener.notify = kwl_expose_notify;
	wl_signal_add(&backend->events.expose, &srv.listener);
	
	wl_display_run(srv.display);

	wl_event_source_remove(key);
	wl_event_source_remove(sigint);
	wl_display_destroy(srv.display);
	
	return 0;

	UNUSED(argc);
	UNUSED(argv);
}
