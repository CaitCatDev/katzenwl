#include "drm_mode.h"
#include "kwl/interfaces/kwl-buffer.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include <wayland-server-core.h>
#include <wayland-util.h>
#include <xf86drm.h>
#include <xf86drmMode.h>

#include <kwl/log/logger.h>
#include <kwl/backend/backend.h>

#include <kwl/interfaces/kwl-output.h>

#include <kwl-private/backend/drm.h>
/* TODO: How are we going to manage these at at the moment it would be a pain
 * to do DRM on it's own or with evdev or etc... basically change this because
 * having these be interlocked could lead to future issues with making it harder 
 * too change the code in any major way which is not good.
 */
#include <libseat.h>
#include <libinput.h>
#include <libudev.h>

#include <sys/mman.h>

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
typedef struct kwl_drm_fb {
	uint32_t handle, fb_id;

	uint32_t height, width;
	uint32_t pitch;
	uint64_t size, offset;
	uint32_t bpp, depth;

	uint32_t *data;

	int fd;
} kwl_drm_fb_t;

typedef struct kwl_drm_output {
	kwl_output_t output;

	drmModeConnectorPtr connector;
	drmModeCrtcPtr saved_crtc;
	kwl_drm_fb_t *fb;
	int shutdown;
	int waiting;
} kwl_drm_output_t;

kwl_drm_fb_t *kwl_drm_backend_create_fb(int fd, uint32_t width, uint32_t height, uint32_t bpp, uint32_t depth) {
	kwl_drm_fb_t *fb;
	int res; 

	fb = calloc(1, sizeof(kwl_drm_fb_t));
	if(!fb) {
		kwl_log_error("Failed to allocate fb structure %m\n");
		goto err_calloc;
	}

	fb->width = width;
	fb->height = height;
	fb->bpp = bpp;
	fb->depth = depth;
	fb->fd = fd;
	
	
	res = drmModeCreateDumbBuffer(fd, width, height, bpp, 0, &fb->handle, &fb->pitch, &fb->size);
	if(res < 0) {
		kwl_log_error("Failed to create drm dumb buffer %m\n");
		goto err_create_dumb;
	}

	res = drmModeMapDumbBuffer(fd, fb->handle, &fb->offset);
	if(res < 0) {
		kwl_log_error("Failed to map dumb buffer %m\n");
		goto err_map_dumb;
	}

	res = drmModeAddFB(fd, width, height, depth, bpp, fb->pitch, fb->handle, &fb->fb_id);
	if(res < 0) {
		kwl_log_error("Failed to add drm fb %m\n");
		goto err_map_dumb;
	}

	fb->data = mmap(NULL, fb->size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, fb->offset);
	if(fb->data == MAP_FAILED) {
		kwl_log_error("Failed to mmap DRM buffer\n");
		goto err_mmap;
	}

	return fb;


err_mmap:
	drmModeRmFB(fd, fb->fb_id);
err_map_dumb:
	drmModeDestroyDumbBuffer(fd, fb->handle);
err_create_dumb:
	free(fb);
err_calloc:
	return NULL;
}

void kwl_drm_backend_destroy_fb(kwl_drm_fb_t *fb) {
	
	munmap(fb->data, fb->size);

	drmModeRmFB(fb->fd, fb->fb_id);
	drmModeDestroyDumbBuffer(fb->fd, fb->handle);

	free(fb);
}

static void handle_enable(struct libseat *backend, void *data) {
	(void)backend;
	kwl_drm_backend_t *drm = data;
	kwl_output_t *output;
	kwl_drm_output_t *drm_out;
	kwl_log_info("Enabling Seat\n");	
	
	wl_list_for_each(output, &drm->outputs, link) {
		drm_out = (void *)output;
		drm_out->shutdown = 0;

		drmModeSetCrtc(drm->fd,  drm_out->saved_crtc->crtc_id, drm_out->fb->fb_id, 0, 0, &drm_out->connector->connector_id, 1, drm_out->connector->modes);	
			
		drmModePageFlip(drm->fd, drm_out->saved_crtc->crtc_id, drm_out->fb->fb_id, DRM_MODE_PAGE_FLIP_EVENT, output);
		drm_out->waiting = 1;
		kwl_log_info("DRM devices enabled\n");
	}
	
	if(drm->input) {
		kwl_log_info("Enabling Input Devices\n");
		libinput_resume(drm->input);
		kwl_log_info("Enabled Seats\n");
	}
	
	drm->active++;
}

static void handle_disable(struct libseat *backend, void *data) {
	(void)backend;
	kwl_drm_backend_t *drm = data;
	kwl_output_t *output;
	kwl_drm_output_t *drm_out;
	drm->active--;
	

	kwl_log_debug("Disabling DRM devices\n");
	wl_list_for_each(output, &drm->outputs, link) {
		drm_out = (void *)output;
		drm_out->shutdown = 1;
	}

	wl_list_for_each(output, &drm->outputs, link) {
		while(drm_out->waiting) {};
	}

	kwl_log_debug("Disabling input devs\n");
	libinput_suspend(drm->input);
	kwl_log_debug("Disabling Seat\n");
	libseat_disable_seat(backend);
	kwl_log_debug("Disabled Seat\n");
}

int kwl_input_seatd_event(int fd, unsigned int mask, void *data) {
	kwl_drm_backend_t *drm = data;
	if(libseat_dispatch(drm->seat, 0) == -1) {
		kwl_log_debug("seatd dispatch error\n");
		wl_display_terminate(drm->display);
	}
	return 0;
}

static kwl_drm_backend_device_t *kwl_drm_find_device_by_fd(int fd, struct wl_list list) {
	kwl_drm_backend_device_t *device;

	wl_list_for_each(device, &list, link) {
		if(device->fd == fd) {
			return device;
		}
	}
	return NULL;
}

static void close_res(int fd, void *data) {
	kwl_drm_backend_device_t *device;
	kwl_drm_backend_t *drm = data;
	printf("Closing Device: %d\n", fd);	
	device = kwl_drm_find_device_by_fd(fd, drm->devices);
	
	libseat_close_device(drm->seat, device->dev);
	close(device->fd);

	wl_list_remove(&device->link);
	free(device);
}

static int open_res(const char *path, int flags, void *data) {
	kwl_drm_backend_device_t *device;
	kwl_drm_backend_t *drm = data;
	device = calloc(1, sizeof(*device));
	
	device->dev = libseat_open_device(drm->seat, path, &device->fd);
	
	wl_list_insert(&drm->devices, &device->link);
	printf("Opened dev: FD %d Dev %d %s\n", device->fd, device->dev, path);
	return device->fd;
}

const static struct libinput_interface libinput_interface = {
	.open_restricted = open_res,
	.close_restricted = close_res,
};

const static struct libseat_seat_listener listener = {
	.enable_seat = handle_enable,
	.disable_seat = handle_disable,
};

int kwl_input_read_event(int fd, unsigned int mask, void *data) {
	struct libinput_event *event;
	enum libinput_event_type type;
	kwl_drm_backend_t *drm = data;
	
	libinput_dispatch(drm->input);
	while((event = libinput_get_event(drm->input)) != NULL) {
		type = libinput_event_get_type(event);
		
		if(type == LIBINPUT_EVENT_KEYBOARD_KEY && libinput_event_keyboard_get_key_state(libinput_event_get_keyboard_event(event)) == LIBINPUT_KEY_STATE_PRESSED) {
			if(0x10 == libinput_event_keyboard_get_key(libinput_event_get_keyboard_event(event))) {	
				wl_display_terminate(drm->display);
			}
			if(0x2 == libinput_event_keyboard_get_key(libinput_event_get_keyboard_event(event))) {	
				libseat_switch_session(drm->seat, 4);
			}			
		}

		libinput_event_destroy(event);
		libinput_dispatch(drm->input);
	}

	return 0;
}

void kwl_drm_backend_start(kwl_backend_t *backend) {
	kwl_drm_backend_t *drm = (void *)backend;
	kwl_output_t *output;
	kwl_drm_output_t *drm_out;
	kwl_log_debug("Starting Backend\n");

	wl_list_for_each(output, &drm->outputs, link) {
		wl_signal_emit(&backend->events.new_output, output);
		drm_out = (void *)output;
		drmModePageFlip(drm->fd, drm_out->saved_crtc->crtc_id, drm_out->fb->fb_id, DRM_MODE_PAGE_FLIP_EVENT, output);
		drm_out->waiting = 1;
	}


	return;
}

void kwl_drm_output_deinit(kwl_drm_output_t *output) {
	drmModeFreeConnector(output->connector);
	free(output->output.make);
	free(output->output.model);

	wl_global_remove(output->output.global);

	free(output);
}

void kwl_drm_backend_deinit(kwl_backend_t *backend) {
	kwl_drm_backend_t *drm = (void *)backend;
	kwl_drm_output_t *drm_out;
	kwl_output_t *output, *tmp;

	wl_list_for_each(output, &drm->outputs, link) {
		drm_out = (void *)output;
		drm_out->shutdown = 1;
	}

	wl_list_for_each_safe(output, tmp, &drm->outputs, link) {
		while(drm_out->waiting) {};
		drmModeSetCrtc(drm->fd, drm_out->saved_crtc->crtc_id, drm_out->saved_crtc->buffer_id, 
				0, 0, &drm_out->connector->connector_id, 1, &drm_out->saved_crtc->mode);

		kwl_drm_backend_destroy_fb(drm_out->fb);
		drmModeFreeCrtc(drm_out->saved_crtc);
		
		wl_list_remove(&output->link);
		kwl_drm_output_deinit(drm_out);
	}

	wl_event_source_remove(drm->drm_ev);
	wl_event_source_remove(drm->seatd_ev);
	wl_event_source_remove(drm->input_ev);

	libinput_unref(drm->input);

	libseat_close_device(drm->seat, drm->dev);

	libseat_close_seat(drm->seat);

	udev_unref(drm->udev);

	free(drm);
}

uint16_t bswap(uint16_t i) {
	uint8_t high = (i >> 8) & 0xff;
	uint8_t low = (i) & 0xff;


	return (low << 8) | high;
}

void parse_edid(void *edid, size_t length, kwl_output_t *output) {
	uint64_t SPECIAL = *((uint64_t*)edid);
	uint16_t manufacter = *((uint16_t*)edid + 4);
	int len;

	if(SPECIAL == 0x00ffffffffffff00) {
		kwl_log_debug("EDID is valid\n");
	}
	
	manufacter = bswap(manufacter);
	uint16_t refresh = ((uint8_t *)edid)[0x41] | ((((uint8_t*)edid)[0x42] & 0x0f) << 7);
	uint8_t letter1 = (manufacter >> 10) + 'A' - 1;
	uint8_t letter2 = ((manufacter >> 5) & 0x1f) + 'A' - 1;
	uint8_t letter3 = ((manufacter) & 0x1f) + 'A' - 1;
	
	/*TODO: Acutal manaufacter names*/
	len = snprintf(NULL, 0, "%c%c%c", letter1, letter2, letter3);

	output->make = calloc(1, len + 1);

	snprintf(output->make, len + 1, "%c%c%c\n", letter1, letter2, letter3);

	/*Currently don't Even attempt to decode the model*/
	len = snprintf(NULL, 0, "0x%x\n",*((uint16_t*)edid + 5)); 

	output->model = calloc(1, len + 1);

	len = snprintf(output->model, len + 1, "0x%x\n",*((uint16_t*)edid + 5)); 

	kwl_log_debug("%c%c%c\n", letter1, letter2, letter3);
	kwl_log_debug("Refresh Rate: %d\n", refresh);
	
}



static void page_flip_handler(int fd, unsigned int frame,
		unsigned int sec, unsigned int usec, void *data) {
	
	kwl_drm_output_t *output = data;
	output->waiting = 0;

	if(output->shutdown == 0) {
		wl_signal_emit(&output->output.events.frame, data);
		if(drmModePageFlip(fd, output->saved_crtc->crtc_id, output->fb->fb_id, DRM_MODE_PAGE_FLIP_EVENT, output)) {
			kwl_log_debug("Error Flipping page\n");
		}
	}
	
}

int kwl_drm_event(int fd, unsigned int mask, void *data) {
	drmEventContext drmev = {
		.version = 2,
		.page_flip_handler = page_flip_handler,
	};

	if(drmHandleEvent(fd, &drmev)) {
	}
	return 0;
}


void kwl_drm_output_commit_buffer(kwl_buffer_t *buffer, kwl_output_t *output) {
	void *data = kwl_buffer_get_data_ptr(buffer);
	kwl_drm_output_t *drm_out = (void*)output;

	memcpy(drm_out->fb->data, data, 4 * output->mode.height * output->mode.width);
}

static const struct kwl_output_implementation drm_output_implementation = { 
	.commit = kwl_drm_output_commit_buffer,
};

kwl_drm_output_t *kwl_drm_output_init(struct wl_display *display) {
	kwl_drm_output_t *output = calloc(1, sizeof(kwl_drm_output_t));
	
	output->output.impl = drm_output_implementation;
	output->output.global = wl_global_create(display, &wl_output_interface, 1, output, kwl_output_bind);
	return output;
}

kwl_backend_t *kwl_drm_backend_init(struct wl_display *display) {
	kwl_drm_backend_t *drm;
	struct wl_event_loop *loop = wl_display_get_event_loop(display);
	drm = calloc(1, sizeof(kwl_drm_backend_t));
	
	drm->udev = udev_new();
	wl_list_init(&drm->devices);
	wl_list_init(&drm->outputs);
	
	drm->seat = libseat_open_seat(&listener, drm);
	while(drm->active == 0) {
		libseat_dispatch(drm->seat, 0);
	}
	drm->seatd_ev = wl_event_loop_add_fd(loop, libseat_get_fd(drm->seat), WL_EVENT_READABLE, kwl_input_seatd_event, drm);

	libseat_set_log_level(LIBSEAT_LOG_LEVEL_INFO);

	kwl_log_debug("Seat: %s active\n", libseat_seat_name(drm->seat));
	//drm->fd = open("/dev/dri/card1", O_RDWR | O_CLOEXEC);
	drm->dev = libseat_open_device(drm->seat, "/dev/dri/card1", &drm->fd);
	kwl_log_debug("libseat open device backend: %p path: %s\nDev %d Fd %d\n",
			drm->seat, "/dev/dri/card1", drm->dev, drm->fd);

	drm->drm_ev = wl_event_loop_add_fd(loop, drm->fd, WL_EVENT_READABLE, kwl_drm_event, drm);

	drmModeResPtr res = drmModeGetResources(drm->fd);
	/*Libinput */
	drm->input = libinput_udev_create_context(&libinput_interface, drm, drm->udev);
	kwl_log_debug("%p %p\n", drm->udev, drm->input);
	libinput_udev_assign_seat(drm->input, libseat_seat_name(drm->seat));

	int fd = libinput_get_fd(drm->input);

	drm->input_ev = wl_event_loop_add_fd(loop, fd, WL_EVENT_READABLE, kwl_input_read_event, drm);


	wl_list_init(&drm->outputs);

	for(uint32_t i = 0; i < res->count_connectors; i++) {
		drmModeConnectorPtr connector = drmModeGetConnector(drm->fd, res->connectors[i]);
		
		if(connector->connection == DRM_MODE_CONNECTED) {
			kwl_drm_output_t *output = kwl_drm_output_init(display);
			output->output.pwidth = connector->mmWidth;
			output->output.pheight = connector->mmHeight;
			output->output.subpix = connector->subpixel;
			output->output.mode.height = connector->modes[0].vdisplay;
			output->output.mode.width = connector->modes[0].hdisplay;
			output->output.mode.flag = 1;
			output->output.mode.refresh = connector->modes[0].vrefresh * 1000;;
			
			wl_signal_init(&output->output.events.frame);
			for(uint32_t i = 0; i < connector->count_props; i++) {
				drmModePropertyPtr prop = drmModeGetProperty(drm->fd, connector->props[i]);
			
				if(strcmp(prop->name, "EDID") == 0) {
					drmModePropertyBlobPtr blob = drmModeGetPropertyBlob(drm->fd, connector->prop_values[i]);
					kwl_log_debug("EDID Blob:");
					for(uint32_t i = 0; i < blob->length; i++) {
						if((i % 20) == 0) {
							kwl_log_printf(KWL_LOG_DEBUG, "\n");
						}
						kwl_log_printf(KWL_LOG_DEBUG, "%.2x ", ((uint8_t*)blob->data)[i]);
					}
					kwl_log_printf(KWL_LOG_DEBUG, "\n");
					parse_edid(blob->data, blob->length, &output->output);
					drmModeFreePropertyBlob(blob);
				}

				drmModeFreeProperty(prop);
			}


			drmModeEncoderPtr enc = drmModeGetEncoder(drm->fd, connector->encoder_id);
			output->saved_crtc = drmModeGetCrtc(drm->fd, enc->crtc_id);
			drmModeFreeEncoder(enc);

			output->connector = connector;
			output->fb = kwl_drm_backend_create_fb(drm->fd, connector->modes->hdisplay, connector->modes->vdisplay, 32, 24);
			memset(output->fb->data, 0xff, output->fb->size);

			drmModeSetCrtc(drm->fd,  output->saved_crtc->crtc_id, output->fb->fb_id, 0, 0, &connector->connector_id, 1, connector->modes);	
			wl_list_insert(&drm->outputs, &output->output.link);
			continue;
		}
		drmModeFreeConnector(connector);
	}

	drmModeFreeResources(res);
	wl_signal_init(&drm->impl.events.new_output);
	drm->impl.callbacks.start = kwl_drm_backend_start;
	drm->impl.callbacks.deinit = kwl_drm_backend_deinit;

	drm->display = display;

	return (void*)drm;
}
