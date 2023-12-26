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
#include <wayland-util.h>


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

	return 0;
	UNUSED(signo);
}


struct wl_global *kwl_compositor_create(struct wl_display *display);


struct kwl_loutput {
	kwl_output_t *output;
	struct kwl_server *server;
	struct wl_listener frame;
	struct wl_list link;
};

struct kwl_server {
	struct wl_display *display;
	kwl_software_renderer_t *renderer;
	struct wl_listener new_output;
	struct wl_list outputs;
};

/*Data is a kwl_output_t that needs rendered to*/ 
void kwl_frame_notify(struct wl_listener *listen, void *data) {
	struct kwl_loutput *loutput = wl_container_of(listen, loutput, frame);
	kwl_output_t *output = data;

	float r = output->color;

	kwl_software_renderer_clear_screen(data, loutput->server->renderer, r, 0.2f, 0.3f);
}

/*Data is the kwl_output_t *structure*/
void kwl_server_add_output(struct wl_listener *listener, void *data) {
	struct kwl_server *srv = wl_container_of(listener, srv, new_output);
	struct kwl_loutput *loutput = calloc(1, sizeof(*loutput));

	kwl_output_t *output = data;
	loutput->output = output;
	loutput->frame.notify = kwl_frame_notify;
	loutput->server = srv;
	output->color = ((float)rand() / (float)(RAND_MAX)) * 1.0f;


	wl_signal_add(&output->events.frame, &loutput->frame);	
	wl_list_insert(&srv->outputs, &loutput->link);

}

int main(int argc, char *argv[]) {
	struct kwl_server srv;
	struct wl_global *compositor;
	kwl_seat_t *seat;
	struct wl_event_loop *loop;
	struct wl_event_source *sigint;
	const char *socket;
	struct kwl_loutput *output, *tmp;
	
	srv.display = wl_display_create();
	
	socket = wl_display_add_socket_auto(srv.display);

	kwl_log_info("%s\n", socket);

	loop = wl_display_get_event_loop(srv.display);

	compositor = kwl_compositor_create(srv.display);
	wl_display_init_shm(srv.display);

	wl_list_init(&srv.outputs);

	//key = wl_event_loop_add_fd(loop, STDIN_FILENO, WL_EVENT_READABLE, stdin_keypress, srv.display);
	sigint = wl_event_loop_add_signal(loop, SIGINT, sigint_handler, srv.display);
	kwl_backend_t *backend = kwl_backend_init_env(srv.display);
	srv.renderer = kwl_software_renderer_init((void *)backend);

	seat = kwl_seat_init(srv.display);

	srv.new_output.notify = kwl_server_add_output;
	wl_signal_add(&backend->events.new_output, &srv.new_output);

	srand(time(NULL));

	kwl_backend_start(backend);
	kwl_log_debug("Running Display\n");
	wl_display_run(srv.display);

	wl_global_remove(compositor);
	wl_global_remove(seat->global);
	free(seat);
	//wl_event_source_remove(key);
	wl_event_source_remove(sigint);

	wl_list_for_each_safe(output, tmp, &srv.outputs, link) {
		free(output);
	}

	kwl_backend_deinit(backend);
	wl_display_destroy(srv.display);
	

	return 0;

	UNUSED(argc);
	UNUSED(argv);
}
