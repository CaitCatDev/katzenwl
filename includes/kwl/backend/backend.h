#pragma once

#include <wayland-server.h>
#include <wayland-server-core.h>
#include <wayland-server-protocol.h>

/** 
 * \file backend.h
 * \author Caitcat
 * \date 6 Dec 2023
 *
 * Generic backend functions to be called so an application
 * can run without knowning the exact backend in use.
 * allowing for multiple backends to be created and easily
 * added on this framework
 */


/**
 * \brief Generic Backend exposed as an opaque pointer.
 *
 *
 * in the public facing side of the library. It is just a
 * pointer to the actual backend structure with callback
 * functions to the real backend functions
 */
typedef struct kwl_backend kwl_backend_t;


typedef void (*kwl_backend_start_t)(kwl_backend_t *backend);
typedef void (*kwl_backend_deinit_t)(kwl_backend_t *backend);

/** \todo at the moment this will cause issues as the
 * expose event will cause issues on multiple output
 * displays as this assumes one output/all outputs 
 * having the same redrawing intervals
 */
struct kwl_backend_events {
	struct wl_signal expose; /**< Redraw output(s) */
};

struct kwl_backend_callbacks {
	kwl_backend_start_t start; /**< Backend Start callback function pointer*/
	kwl_backend_deinit_t deinit; /**< Backend deinit callback function pointer*/
};

struct kwl_backend {
	struct kwl_backend_callbacks callbacks;
	struct kwl_backend_events events;
};

/*Functions*/

/** \brief Get backend from name.
 *
 * This function takes a name and returns a 
 * backend for that name if no backend with
 * that name is supported it returns NULL.
 *
 * \param name the name of the backend to use.
 * e.g. (XCB, etc...)
 * \param display the wayland display of this server
 *
 * \retval NULL Function Error
 * \retval !NULL Pointer to backend
 */
kwl_backend_t *kwl_backend_init_name(const char *name, struct wl_display *display);


/** \brief Get backend from Environment Vars.
 *
 * This function calls a backend creation function
 * Based on the currently set environment variables
 *
 * \retval NULL Function Error
 * \retval !NULL Pointer to backend
 */
kwl_backend_t *kwl_backend_init_env(struct wl_display *display);

/*Call into the backend functions*/

/** \brief Start the backend 
 *	\param backend to start
 */
void kwl_backend_start(kwl_backend_t *backend);


/** \brief deinit the backend
 *	\param backend to deinit and clean up
 */
void kwl_backend_deinit(kwl_backend_t *backend);
