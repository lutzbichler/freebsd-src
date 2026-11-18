/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2026 The FreeBSD Foundation
 *
 * This software was developed by Aymeric Wibo <obiwac@freebsd.org>
 * under sponsorship from the FreeBSD Foundation.
 */

/*
 * Interrupts from ACPI devices outside the consumer's bus ancestry on x86.
 * Both sides register independently and remain registered until detached.
 */

#include <sys/param.h>
#include <sys/bus.h>
#include <sys/kernel.h>
#include <sys/malloc.h>
#include <sys/queue.h>
#include <sys/sx.h>
#include <sys/systm.h>

#include <contrib/dev/acpica/include/acpi.h>
#include <dev/acpica/acpivar.h>

#ifdef INTRNG
#error "INTRNG supported; you should not be using this!"
#endif

struct acpi_cousin_intr_request {
	TAILQ_ENTRY(acpi_cousin_intr_request) link;
	device_t consumer;
	acpi_cousin_intr_setup_cb_t setup;
	acpi_cousin_intr_detach_cb_t detach;
	acpi_cousin_intr_intr_cb_t intr;
	struct acpi_cousin_intr_source *source;
};

struct acpi_cousin_intr_source {
	TAILQ_ENTRY(acpi_cousin_intr_source) link;
	device_t provider;
	ACPI_HANDLE consumer;
	void *arg;
	acpi_cousin_intr_activate_cb_t activate;
	acpi_cousin_intr_deactivate_cb_t deactivate;
	struct acpi_cousin_intr_request *request;
};

/* Callbacks may sleep and temporarily drop the bus topology lock (Giant). */
static struct sx cousin_intr_lock;
SX_SYSINIT(cousin_intr_lock, &cousin_intr_lock, "ACPI cousin interrupts");
static TAILQ_HEAD(, acpi_cousin_intr_request) requests =
    TAILQ_HEAD_INITIALIZER(requests);
static TAILQ_HEAD(, acpi_cousin_intr_source) sources =
    TAILQ_HEAD_INITIALIZER(sources);

static bool
acpi_cousin_intr_matches(struct acpi_cousin_intr_request *req,
    struct acpi_cousin_intr_source *source)
{
	return (source->request == NULL && req->source == NULL &&
	    acpi_get_handle(req->consumer) == source->consumer);
}

/* The provider must undo any partial setup if activation fails. */
static bool
acpi_cousin_intr_connect(struct acpi_cousin_intr_request *req,
    struct acpi_cousin_intr_source *source)
{
	int error;

	if (!acpi_cousin_intr_matches(req, source))
		return (false);

	/* The handler may run as soon as the provider installs it. */
	source->request = req;
	req->source = source;
	error = source->activate(source->arg, source);
	if (error != 0) {
		req->source = NULL;
		source->request = NULL;
		return (false);
	}
	/* Do not switch the consumer out of polling until setup succeeds. */
	req->setup(req->consumer);
	return (true);
}

static void
acpi_cousin_intr_match_request(struct acpi_cousin_intr_request *req)
{
	struct acpi_cousin_intr_source *source;

	TAILQ_FOREACH(source, &sources, link) {
		if (acpi_cousin_intr_connect(req, source))
			break;
	}
}

static void
acpi_cousin_intr_match_source(struct acpi_cousin_intr_source *source)
{
	struct acpi_cousin_intr_request *req;

	TAILQ_FOREACH(req, &requests, link) {
		if (acpi_cousin_intr_connect(req, source))
			break;
	}
}

struct acpi_cousin_intr_request *
acpi_cousin_intr_req(device_t consumer, acpi_cousin_intr_setup_cb_t setup,
    acpi_cousin_intr_detach_cb_t detach, acpi_cousin_intr_intr_cb_t intr)
{
	struct acpi_cousin_intr_request *req;

	bus_topo_assert();
	req = malloc(sizeof(*req), M_DEVBUF, M_WAITOK | M_ZERO);
	req->consumer = consumer;
	req->setup = setup;
	req->detach = detach;
	req->intr = intr;
	sx_xlock(&cousin_intr_lock);
	TAILQ_INSERT_TAIL(&requests, req, link);
	acpi_cousin_intr_match_request(req);
	sx_xunlock(&cousin_intr_lock);
	return (req);
}

bool
acpi_cousin_intr_connected(struct acpi_cousin_intr_request *req)
{
	bool connected;

	bus_topo_assert();
	sx_xlock(&cousin_intr_lock);
	connected = req->source != NULL;
	sx_xunlock(&cousin_intr_lock);
	return (connected);
}

/* Stop and drain the provider before clearing or freeing the request. */
int
acpi_cousin_intr_cancel(struct acpi_cousin_intr_request *req)
{
	struct acpi_cousin_intr_source *source;
	int error;

	bus_topo_assert();
	if (req == NULL)
		return (0);
	sx_xlock(&cousin_intr_lock);
	source = req->source;
	if (source != NULL) {
		error = source->deactivate(source->arg);
		if (error != 0) {
			sx_xunlock(&cousin_intr_lock);
			return (error);
		}
		source->request = NULL;
		req->source = NULL;
	}
	TAILQ_REMOVE(&requests, req, link);
	free(req, M_DEVBUF);
	if (source != NULL)
		acpi_cousin_intr_match_source(source);
	sx_xunlock(&cousin_intr_lock);
	return (0);
}

struct acpi_cousin_intr_source *
acpi_cousin_intr_provide(device_t provider, ACPI_HANDLE consumer, void *arg,
    acpi_cousin_intr_activate_cb_t activate,
    acpi_cousin_intr_deactivate_cb_t deactivate)
{
	struct acpi_cousin_intr_source *source;

	bus_topo_assert();
	source = malloc(sizeof(*source), M_DEVBUF, M_WAITOK | M_ZERO);
	source->provider = provider;
	source->consumer = consumer;
	source->arg = arg;
	source->activate = activate;
	source->deactivate = deactivate;
	sx_xlock(&cousin_intr_lock);
	TAILQ_INSERT_TAIL(&sources, source, link);
	acpi_cousin_intr_match_source(source);
	sx_xunlock(&cousin_intr_lock);
	return (source);
}

int
acpi_cousin_intr_unprovide(struct acpi_cousin_intr_source *source)
{
	struct acpi_cousin_intr_request *req;
	int error;

	bus_topo_assert();
	if (source == NULL)
		return (0);
	sx_xlock(&cousin_intr_lock);
	req = source->request;
	if (req != NULL) {
		error = source->deactivate(source->arg);
		if (error != 0) {
			sx_xunlock(&cousin_intr_lock);
			return (error);
		}
		req->source = NULL;
		source->request = NULL;
	}
	TAILQ_REMOVE(&sources, source, link);
	free(source, M_DEVBUF);
	if (req != NULL) {
		req->detach(req->consumer);
		acpi_cousin_intr_match_request(req);
	}
	sx_xunlock(&cousin_intr_lock);
	return (0);
}

/* The provider must drain its handler before unregistering the source. */
void
acpi_cousin_intr_trigger(struct acpi_cousin_intr_source *source)
{
	struct acpi_cousin_intr_request *req;

	req = source->request;
	if (req != NULL)
		req->intr(req->consumer);
}