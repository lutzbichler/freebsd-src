/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2026 Beckhoff Automation GmbH & Co. KG
 */

#ifndef _DEV_GPIO_INTEL_INTELGPIO_H_
#define _DEV_GPIO_INTEL_INTELGPIO_H_

#include <sys/param.h>
#include <sys/bus.h>
#include <sys/gpio.h>
#include <sys/kernel.h>

#include <contrib/dev/acpica/include/acpi.h>

#define INTELGPIO_PADBAR_REG			0x00c
#define INTELGPIO_PAD_SIZE			16

#define INTELGPIO_PADCFG0_GPIOTXSTATE		(1 << 0)
#define INTELGPIO_PADCFG0_GPIORXSTATE		(1 << 1)
#define INTELGPIO_PADCFG0_GPIOTXDIS		(1 << 8)
#define INTELGPIO_PADCFG0_GPIORXDIS		(1 << 9)
#define INTELGPIO_PADCFG0_PMODE_MASK		(0xf << 10)
#define INTELGPIO_PADCFG0_PMODE_GPIO		0
#define INTELGPIO_PADCFG0_GPIROUTNMI		(1U << 17)
#define INTELGPIO_PADCFG0_GPIROUTSMI		(1U << 18)
#define INTELGPIO_PADCFG0_GPIROUTSCI		(1U << 19)
#define INTELGPIO_PADCFG0_GPIROUTIOXAPIC	(1U << 20)
#define INTELGPIO_PADCFG0_RXINV			(1U << 23)
#define INTELGPIO_PADCFG0_RXEVCFG_MASK		(3U << 25)
#define INTELGPIO_PADCFG0_RXEVCFG_LEVEL		(0U << 25)
#define INTELGPIO_PADCFG0_RXEVCFG_EDGE		(1U << 25)
#define INTELGPIO_PADCFG0_RXEVCFG_BOTH		(3U << 25)

#define INTELGPIO_PADCFG1			0x004
#define INTELGPIO_PADCFG1_TERM_MASK		(7U << 10)
#define INTELGPIO_PADCFG1_TERM_5K		(2U << 10)
#define INTELGPIO_PADCFG1_TERM_UP		(1U << 13)

#define INTELGPIO_REVID_REG			0x000
#define INTELGPIO_REVID_SHIFT			16
#define INTELGPIO_REVID_3BIT_PADOWN		0x110

#define INTELGPIO_GPIO_NOMAP			(-1)
#define INTELGPIO_MAX_COMMUNITIES		8

struct intelgpio_padgroup {
	int first_pad;
	int npads;
	int gpio_base;
	const char *name;

	/* used when using interrupts */
	unsigned int reg_num;
	unsigned int padown_num;
};

struct intelgpio_community {
	int ngroups;
	const struct intelgpio_padgroup *groups;

	/* set/used when using interrupts */
	bool has_intr;
	bus_size_t padown_offset;
	bus_size_t padcfglock_offset;
	bus_size_t hostown_offset;
	bus_size_t is_offset;
	bus_size_t ie_offset;
};

struct intelgpio_platform {
	const struct intelgpio_community *communities;
	int ncommunities;
	char **hids;
	const char *desc;
};

struct intelgpio_softc {
	device_t sc_dev;
	device_t sc_busdev;
	struct mtx sc_mtx;
	const struct intelgpio_platform *sc_plat;

	struct resource *sc_mem_res[INTELGPIO_MAX_COMMUNITIES];
	uint32_t sc_padbar[INTELGPIO_MAX_COMMUNITIES];
	uint32_t sc_revision[INTELGPIO_MAX_COMMUNITIES];
	uint32_t *sc_intr_unmasked[INTELGPIO_MAX_COMMUNITIES];
	struct resource *sc_irq_res;
	int sc_irq_rid;
	void *sc_irq_cookie;
	bool sc_intr_ready;
};

DECLARE_CLASS(intelgpio_driver);

int intelgpio_probe(device_t dev, const struct intelgpio_platform *plat);
int intelgpio_attach(device_t dev, const struct intelgpio_platform *plat);

#endif /* _DEV_GPIO_INTEL_INTELGPIO_H_ */
