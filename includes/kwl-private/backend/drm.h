#pragma once

#include <wayland-server-core.h>

#include <wayland-util.h>
#include <xf86drm.h>
#include <xf86drmMode.h>

#include <kwl/interfaces/kwl-output.h>
#include <kwl/backend/backend.h>

/* TODO: We should change the output structure 
 * as at the moment it doesn't assume to much
 * about the backend it but it needs to use some
 * backend stuff.
 */
#include <kwl/interfaces/kwl-output.h>

#include <libinput.h>
#include <libseat.h>
#include <libudev.h>

/**TODO:
 * So we need move over to libseat_open_device
 * for input devices as at the moment the 
 * we just call open and this is an issue. as it needs
 * the user to be allowed access to those files on the 
 * system. But that isn't great as for example all it 
 * would take it is someone opening a wayland client
 * getting the keymap then opening /dev/input/eventN
 * (The keyboard) to make a rudimentry keylogger.
 * So we need to store the dev id returned by seatd open
 * as we can't just call close we need to call 
 * seatd_close_device which needs the dev id and not the fd
 *
 *
 * So we will copy for now not sure if this is permanent or not
 * but we will use the same basic way we keep tracks of outputs
 * a structure with a wl_list in it.
 */
typedef struct kwl_drm_backend_device {
	int dev;
	int fd; /*FD is all that makes sense to search for as 
			  it's the only constant in both functions 
			*/
	char *str;
	struct wl_list link;
} kwl_drm_backend_device_t;

/** 
 * \file drm.h
 * \author Caitcat
 * \date 6 Dec 2023
 *
 * Generic drm backend functions internal structures and 
 * typedefs this is not intended for use for external 
 * apps using this lib.
 *
 * \internal
 */

typedef struct kwl_drm_backend {
	kwl_backend_t impl;
	
	int fd, dev;
	
	struct udev *udev;
	struct libinput *input;
	struct libseat *seat;
	int active;

	struct wl_list outputs;
	struct wl_list devices;

	struct wl_display *display;
	struct wl_event_source *input_ev;
	struct wl_event_source *seatd_ev;
	struct wl_event_source *drm_ev;
} kwl_drm_backend_t;

kwl_backend_t *kwl_drm_backend_init(struct wl_display *display);
