#include <stdint.h>
#include <stdlib.h>
#include <wayland-server.h>
#include <wayland-server-core.h>
#include <wayland-server-protocol.h>

#include <kwl/log/logger.h>
#include <kwl/interfaces/kwl-seat.h>
#include <kwl/interfaces/kwl-output.h>
#include <kwl/backend/backend.h>


#include <kwl-private/util/macros.h>
#include <kwl-private/renderer/software.h>

#include <unistd.h>
#include <signal.h>


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

struct kwl_server {
	struct wl_display *display;
	kwl_software_renderer_t *renderer;
	struct wl_listener frame;
	struct wl_listener new_output;
};

/*Data is a kwl_output_t that needs rendered to*/ 
void kwl_frame_notify(struct wl_listener *listen, void *data) {
	struct kwl_server *srv = wl_container_of(listen, srv, frame);
	kwl_output_t *output = data;

	float r = output->color;;

	kwl_software_renderer_clear_screen(data, srv->renderer, r, 0.2f, 0.3f);
}

/*Data is the kwl_output_t *structure*/
void kwl_server_add_output(struct wl_listener *listener, void *data) {
	struct kwl_server *srv = wl_container_of(listener, srv, new_output);
	
	kwl_output_t *output = data;
	output->color = ((float)rand() / (float)(RAND_MAX)) * 1.0f;
	kwl_log_debug("%f\n", output->color);	
	wl_signal_add(&output->events.frame, &srv->frame);	
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
	kwl_backend_t *backend = kwl_backend_init_env(srv.display);
	srv.renderer = kwl_software_renderer_init((void *)backend);
	

	kwl_seat_init(srv.display);
	
	srv.frame.notify = kwl_frame_notify;

	srv.new_output.notify = kwl_server_add_output;
	wl_signal_add(&backend->events.new_output, &srv.new_output);
	
	srand(time(NULL));

	kwl_backend_start(backend);

	wl_display_run(srv.display);

	wl_event_source_remove(key);
	wl_event_source_remove(sigint);
	kwl_backend_deinit(backend);
	wl_display_destroy(srv.display);
	return 0;

	UNUSED(argc);
	UNUSED(argv);
}
