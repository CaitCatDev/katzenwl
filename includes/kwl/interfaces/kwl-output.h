#pragma once

#include <stdint.h>
#include <wayland-server.h>
#include <wayland-server-protocol.h>
#include <wayland-server-core.h>

typedef struct {
	enum wl_output_mode flag;
	int32_t width, height;
	int32_t refresh;
} kwl_output_mode_t;

typedef struct {
	char *make;
	char *model;

	int32_t pwidth, pheight;
	int32_t x, y;

	int32_t subpix, transform; /*TODO ENUMS*/

	char *name;
	
	kwl_output_mode_t mode;
} kwl_output_t;


kwl_output_t *kwl_output_init(struct wl_display *display);	
