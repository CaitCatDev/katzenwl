#pragma once

#include <wayland-server-core.h>

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
	
	int fd;
	

} kwl_drm_backend_t;

kwl_backend_t *kwl_drm_backend_init(struct wl_display *display);
