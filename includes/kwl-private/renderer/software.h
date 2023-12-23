#pragma once

#include <kwl-private/backend/xcb.h>

typedef struct kwl_software_renderer {
	kwl_backend_t *backend;
} kwl_software_renderer_t;

void *kwl_software_renderer_init(kwl_backend_t *backend);
void kwl_software_renderer_clear_screen(kwl_output_t *output, kwl_software_renderer_t *renderer, float r, float g, float b);	
