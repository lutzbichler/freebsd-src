/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2026 Beckhoff Automation GmbH & Co. KG
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/bus.h>
#include <sys/gpio.h>
#include <sys/interrupt.h>
#include <sys/kernel.h>
#include <sys/malloc.h>
#include <sys/module.h>
#include <sys/rman.h>

#include <machine/bus.h>
#include <machine/resource.h>

#include <contrib/dev/acpica/include/acpi.h>
#include <contrib/dev/acpica/include/accommon.h>

#include <dev/acpica/acpivar.h>
#include <dev/gpio/gpiobusvar.h>

#include "intelgpio.h"

#include "gpio_if.h"
#include "opt_acpi.h"

#define INTELGPIO_LOCK(_sc)   mtx_lock_spin(&(_sc)->sc_mtx)
#define INTELGPIO_UNLOCK(_sc) mtx_unlock_spin(&(_sc)->sc_mtx)
#define INTELGPIO_LOCK_INIT(_sc)                                     \
	mtx_init(&(_sc)->sc_mtx, device_get_nameunit((_sc)->sc_dev), \
	    "intelgpio", MTX_SPIN)
#define INTELGPIO_LOCK_DESTROY(_sc) mtx_destroy(&(_sc)->sc_mtx)

struct intelgpio_intr_pin {
	const struct intelgpio_community *com;
	const struct intelgpio_padgroup *group;
	struct resource *res;
	bus_size_t pad_off;
	bus_size_t is_off;
	bus_size_t ie_off;
	uint32_t bit;
	int community;
	int group_index;
};

static bool
intelgpio_reg_valid(struct resource *res, bus_size_t off)
{
	return (res != NULL && (off & 3) == 0 &&
	    rman_get_size(res) >= sizeof(uint32_t) &&
	    off <= rman_get_size(res) - sizeof(uint32_t));
}

static int
intelgpio_gpio_to_pad(struct intelgpio_softc *sc, uint32_t pin,
    struct resource **res, bus_size_t *pad_off,
    const struct intelgpio_padgroup **pgp)
{
	const struct intelgpio_platform *plat = sc->sc_plat;
	const struct intelgpio_community *com;
	const struct intelgpio_padgroup *pg;
	int i, j, offset, local_pad;

	for (i = 0; i < plat->ncommunities; i++) {
		com = &plat->communities[i];
		for (j = 0; j < com->ngroups; j++) {
			pg = &com->groups[j];
			if (pg->gpio_base < 0 || pg->npads <= 0)
				continue;
			if (pin >= (uint32_t)pg->gpio_base &&
			    pin - pg->gpio_base < (uint32_t)pg->npads) {
				if (res != NULL)
					*res = sc->sc_mem_res[i];
				if (pad_off != NULL) {
					offset = pin - pg->gpio_base;
					local_pad = (pg->first_pad + offset) -
					    com->groups[0].first_pad;
					if (local_pad < 0)
						return (-1);
					*pad_off = sc->sc_padbar[i] +
					    ((bus_size_t)local_pad * INTELGPIO_PAD_SIZE);
					if (!intelgpio_reg_valid(sc->sc_mem_res[i], *pad_off))
						return (-1);
				}
				if (pgp != NULL)
					*pgp = pg;
				return (0);
			}
		}
	}
	return (-1);
}

/* Translate an exposed GPIO number to the community's interrupt registers. */
static bool
intelgpio_intr_pin(struct intelgpio_softc *sc, uint32_t pin,
    struct intelgpio_intr_pin *ip)
{
	const struct intelgpio_community *com;
	const struct intelgpio_padgroup *pg;
	struct resource *res;
	bus_size_t pad_off, reg_off;
	int i, j;

	if (intelgpio_gpio_to_pad(sc, pin, &res, &pad_off, &pg) != 0 ||
	    !intelgpio_reg_valid(res, pad_off))
		return (false);

	for (i = 0; i < sc->sc_plat->ncommunities; i++) {
		com = &sc->sc_plat->communities[i];
		for (j = 0; j < com->ngroups; j++) {
			if (&com->groups[j] != pg)
				continue;
			if (!com->has_intr || sc->sc_intr_unmasked[i] == NULL ||
			    sc->sc_revision[i] == 0xffff ||
			    pg->npads <= 0 || pg->npads > 32 ||
			    com->is_offset == 0 || com->ie_offset == 0)
				return (false);
			reg_off = (bus_size_t)pg->reg_num * sizeof(uint32_t);
			if (!intelgpio_reg_valid(res, com->is_offset + reg_off) ||
			    !intelgpio_reg_valid(res, com->ie_offset + reg_off))
				return (false);
			ip->com = com;
			ip->group = pg;
			ip->res = res;
			ip->pad_off = pad_off;
			ip->is_off = com->is_offset + reg_off;
			ip->ie_off = com->ie_offset + reg_off;
			ip->bit = 1U << (pin - pg->gpio_base);
			ip->community = i;
			ip->group_index = j;
			return (true);
		}
	}
	return (false);
}

/* Caller holds sc_mtx.  ACPI-owned pads cannot generate GPIO IRQs. */
static bool
intelgpio_intr_pin_usable(struct intelgpio_softc *sc,
	    const struct intelgpio_intr_pin *ip, uint32_t pin, bool irq)
{
	const struct intelgpio_community *com;
	const struct intelgpio_padgroup *pg;
	bus_size_t off;
	unsigned int group_pin, local_pad;
	uint32_t val;

	com = ip->com;
	pg = ip->group;
	group_pin = pin - pg->gpio_base;
	local_pad = pg->first_pad + group_pin - com->groups[0].first_pad;
	if (com->padown_offset != 0) {
		if (!com->has_intr &&
			sc->sc_revision[ip->community] >= INTELGPIO_REVID_3BIT_PADOWN) {
			off = com->padown_offset + (bus_size_t)local_pad * 4;
			if (!intelgpio_reg_valid(ip->res, off) ||
			    (bus_read_4(ip->res, off) & 7U) != 0)
				return (false);
		} else {
			off = com->padown_offset +
			    ((bus_size_t)pg->padown_num + group_pin / 8) * 4;
			if (!intelgpio_reg_valid(ip->res, off))
				return (false);
			val = bus_read_4(ip->res, off);
			if ((val & (0xfU << ((group_pin % 8) * 4))) != 0)
				return (false);
		}
	}
	if (irq && com->hostown_offset != 0) {
		off = com->hostown_offset + (bus_size_t)pg->reg_num * 4;
		if (!intelgpio_reg_valid(ip->res, off) ||
		    (bus_read_4(ip->res, off) & ip->bit) == 0)
			return (false);
	}
	if (com->padcfglock_offset != 0) {
		off = com->padcfglock_offset + (bus_size_t)pg->reg_num * 8;
		if (!intelgpio_reg_valid(ip->res, off) ||
		    (bus_read_4(ip->res, off) & ip->bit) != 0)
			return (false);
	}
	return (true);
}

static device_t
intelgpio_get_bus(device_t dev)
{
	struct intelgpio_softc *sc;

	sc = device_get_softc(dev);
	return (sc->sc_busdev);
}

static int
intelgpio_pin_max(device_t dev, int *maxpin)
{
	struct intelgpio_softc *sc;
	const struct intelgpio_platform *plat;
	const struct intelgpio_community *com;
	const struct intelgpio_padgroup *pg;
	int i, j;

	if (maxpin == NULL)
		return (EINVAL);

	sc = device_get_softc(dev);
	plat = sc->sc_plat;

	/* Groups need not be ordered by GPIO base. */
	*maxpin = 0;
	for (i = 0; i < plat->ncommunities; i++) {
		com = &plat->communities[i];
		for (j = 0; j < com->ngroups; j++) {
			pg = &com->groups[j];
			if (pg->gpio_base == INTELGPIO_GPIO_NOMAP)
				continue;
			*maxpin = MAX(*maxpin, pg->gpio_base + pg->npads - 1);
		}
	}
	return (0);
}

static int
intelgpio_pin_getname(device_t dev, uint32_t pin, char *name)
{
	struct intelgpio_softc *sc;
	const struct intelgpio_padgroup *pg;
	int offset;

	sc = device_get_softc(dev);
	if (intelgpio_gpio_to_pad(sc, pin, NULL, NULL, &pg) != 0)
		return (EINVAL);

	offset = pin - pg->gpio_base;
	snprintf(name, GPIOMAXNAME, "%s%d", pg->name, offset);
	return (0);
}

static int
intelgpio_pin_getcaps(device_t dev, uint32_t pin, uint32_t *caps)
{
	struct intelgpio_softc *sc;
	struct intelgpio_intr_pin ip;
	struct resource *res;
	bus_size_t pad_off;
	bool usable;

	sc = device_get_softc(dev);

	/*
	 * The getcaps function is expected to return the capabilities
	 * of the pin. Validate that the pin number maps to an existing
	 * pad first.
	 */
	if (intelgpio_gpio_to_pad(sc, pin, &res, &pad_off, NULL) != 0 ||
	    res == NULL)
		return (EINVAL);

	*caps = GPIO_PIN_INPUT | GPIO_PIN_OUTPUT;
	if (intelgpio_reg_valid(res, pad_off + INTELGPIO_PADCFG1))
		*caps |= GPIO_PIN_PULLUP | GPIO_PIN_PULLDOWN;
	if (sc->sc_irq_cookie != NULL && intelgpio_intr_pin(sc, pin, &ip)) {
		INTELGPIO_LOCK(sc);
		usable = intelgpio_intr_pin_usable(sc, &ip, pin, true);
		INTELGPIO_UNLOCK(sc);
		if (usable)
			*caps |= GPIO_INTR_MODE_MASK;
	}
	return (0);
}

static int
intelgpio_pin_getflags(device_t dev, uint32_t pin, uint32_t *flags)
{
	struct intelgpio_softc *sc;
	struct resource *res;
	bus_size_t pad_off;
	uint32_t val, term;

	sc = device_get_softc(dev);
	if (intelgpio_gpio_to_pad(sc, pin, &res, &pad_off, NULL) != 0)
		return (EINVAL);
	if (res == NULL)
		return (EINVAL);

	*flags = 0;

	INTELGPIO_LOCK(sc);
	val = bus_read_4(res, pad_off);
	term = intelgpio_reg_valid(res, pad_off + INTELGPIO_PADCFG1) ?
	    bus_read_4(res, pad_off + INTELGPIO_PADCFG1) : 0;
	INTELGPIO_UNLOCK(sc);

	/*
	 * If the pad is not in GPIO mode (i.e. configured for a native
	 * function like PCIe, UART, I2C, etc.), report no flags. The pin
	 * cannot be used as GPIO without first switching it via setflags.
	 */
	if ((val & INTELGPIO_PADCFG0_PMODE_MASK) !=
	    INTELGPIO_PADCFG0_PMODE_GPIO)
		return (0);

	/*
	 * Report OUTPUT only if TX is explicitly enabled. Default to INPUT
	 * otherwise: a non-driving pin is considered an input from the user's
	 * perspective.
	 */
	if (!(val & INTELGPIO_PADCFG0_GPIOTXDIS))
		*flags = GPIO_PIN_OUTPUT;
	else
		*flags = GPIO_PIN_INPUT;
	if ((term & INTELGPIO_PADCFG1_TERM_MASK) != 0) {
		if ((term & INTELGPIO_PADCFG1_TERM_UP) != 0)
			*flags |= GPIO_PIN_PULLUP;
		else
			*flags |= GPIO_PIN_PULLDOWN;
	}

	return (0);
}

static int
intelgpio_pin_setflags(device_t dev, uint32_t pin, uint32_t flags)
{
	struct intelgpio_softc *sc;
	struct intelgpio_intr_pin ip;
	struct resource *res;
	bus_size_t pad_off;
	uint32_t val, term;
	bool check_owner;

	sc = device_get_softc(dev);
	if (intelgpio_gpio_to_pad(sc, pin, &res, &pad_off, NULL) != 0)
		return (EINVAL);
	if (res == NULL)
		return (EINVAL);

	/*
	 * We currently only support input and output, not both simultaneously
	 */
	if (flags & ~(GPIO_PIN_INPUT | GPIO_PIN_OUTPUT | GPIO_PIN_PULLUP |
	    GPIO_PIN_PULLDOWN))
		return (EINVAL);
	if ((flags & GPIO_PIN_INPUT) && (flags & GPIO_PIN_OUTPUT))
		return (EINVAL);
	if ((flags & (GPIO_PIN_PULLUP | GPIO_PIN_PULLDOWN)) ==
	    (GPIO_PIN_PULLUP | GPIO_PIN_PULLDOWN))
		return (EINVAL);
	if ((flags & (GPIO_PIN_PULLUP | GPIO_PIN_PULLDOWN)) != 0 &&
	    !intelgpio_reg_valid(res, pad_off + INTELGPIO_PADCFG1))
		return (EINVAL);

	check_owner = intelgpio_intr_pin(sc, pin, &ip);
	INTELGPIO_LOCK(sc);
	if (check_owner && !intelgpio_intr_pin_usable(sc, &ip, pin, false)) {
		INTELGPIO_UNLOCK(sc);
		return (EPERM);
	}
	val = bus_read_4(res, pad_off);

	/*
	 * Switch the pad to GPIO mode. Intel pads can operate in different
	 * modes (native function 1-15 or GPIO). Since setflags is only called
	 * when userspace or a consumer explicitly wants to use the pin as a
	 * GPIO, we force the pad into GPIO mode here so the direction bits
	 * (TXDIS/RXDIS) below take effect.
	 */
	val &= ~INTELGPIO_PADCFG0_PMODE_MASK;
	val |= INTELGPIO_PADCFG0_PMODE_GPIO;

	if (flags & GPIO_PIN_INPUT)
		val &= ~INTELGPIO_PADCFG0_GPIORXDIS;
	else
		val |= INTELGPIO_PADCFG0_GPIORXDIS;

	if (flags & GPIO_PIN_OUTPUT)
		val &= ~INTELGPIO_PADCFG0_GPIOTXDIS;
	else
		val |= INTELGPIO_PADCFG0_GPIOTXDIS;

	bus_write_4(res, pad_off, val);
	/* With no explicit pull request, retain the firmware setting. */
	if ((flags & (GPIO_PIN_PULLUP | GPIO_PIN_PULLDOWN)) != 0) {
		term = bus_read_4(res, pad_off + INTELGPIO_PADCFG1);
		term &= ~(INTELGPIO_PADCFG1_TERM_MASK | INTELGPIO_PADCFG1_TERM_UP);
		term |= INTELGPIO_PADCFG1_TERM_5K;
		if ((flags & GPIO_PIN_PULLUP) != 0)
			term |= INTELGPIO_PADCFG1_TERM_UP;
		bus_write_4(res, pad_off + INTELGPIO_PADCFG1, term);
	}
	INTELGPIO_UNLOCK(sc);

	return (0);
}

static int
intelgpio_pin_get(device_t dev, uint32_t pin, unsigned int *value)
{
	struct intelgpio_softc *sc;
	struct resource *res;
	bus_size_t pad_off;
	uint32_t val;

	sc = device_get_softc(dev);
	if (intelgpio_gpio_to_pad(sc, pin, &res, &pad_off, NULL) != 0)
		return (EINVAL);
	if (res == NULL)
		return (EINVAL);

	INTELGPIO_LOCK(sc);
	val = bus_read_4(res, pad_off);
	INTELGPIO_UNLOCK(sc);

	*value = (val & INTELGPIO_PADCFG0_GPIORXSTATE) ?
	    GPIO_PIN_HIGH : GPIO_PIN_LOW;

	return (0);
}

static int
intelgpio_pin_set(device_t dev, uint32_t pin, unsigned int value)
{
	struct intelgpio_softc *sc;
	struct resource *res;
	bus_size_t pad_off;
	uint32_t val;

	sc = device_get_softc(dev);
	if (intelgpio_gpio_to_pad(sc, pin, &res, &pad_off, NULL) != 0)
		return (EINVAL);
	if (res == NULL)
		return (EINVAL);

	INTELGPIO_LOCK(sc);
	val = bus_read_4(res, pad_off);
	if (value == GPIO_PIN_LOW)
		val &= ~INTELGPIO_PADCFG0_GPIOTXSTATE;
	else
		val |= INTELGPIO_PADCFG0_GPIOTXSTATE;
	bus_write_4(res, pad_off, val);
	INTELGPIO_UNLOCK(sc);

	return (0);
}

static int
intelgpio_pin_toggle(device_t dev, uint32_t pin)
{
	struct intelgpio_softc *sc;
	struct resource *res;
	bus_size_t pad_off;
	uint32_t val;

	sc = device_get_softc(dev);
	if (intelgpio_gpio_to_pad(sc, pin, &res, &pad_off, NULL) != 0)
		return (EINVAL);
	if (res == NULL)
		return (EINVAL);

	INTELGPIO_LOCK(sc);
	val = bus_read_4(res, pad_off);
	val ^= INTELGPIO_PADCFG0_GPIOTXSTATE;
	bus_write_4(res, pad_off, val);
	INTELGPIO_UNLOCK(sc);

	return (0);
}

static void
intelgpio_pin_config_intr(device_t dev, uint32_t pin, uint32_t mode)
{
	struct intelgpio_softc *sc = device_get_softc(dev);
	struct intelgpio_intr_pin ip;
	uint32_t cfg, event, inv;

	switch (mode) {
	case GPIO_INTR_LEVEL_HIGH:
		event = INTELGPIO_PADCFG0_RXEVCFG_LEVEL;
		inv = 0;
		break;
	case GPIO_INTR_LEVEL_LOW:
		event = INTELGPIO_PADCFG0_RXEVCFG_LEVEL;
		inv = INTELGPIO_PADCFG0_RXINV;
		break;
	case GPIO_INTR_EDGE_RISING:
		event = INTELGPIO_PADCFG0_RXEVCFG_EDGE;
		inv = 0;
		break;
	case GPIO_INTR_EDGE_FALLING:
		event = INTELGPIO_PADCFG0_RXEVCFG_EDGE;
		inv = INTELGPIO_PADCFG0_RXINV;
		break;
	case GPIO_INTR_EDGE_BOTH:
		event = INTELGPIO_PADCFG0_RXEVCFG_BOTH;
		inv = 0;
		break;
	default:
		device_printf(dev, "invalid interrupt mode %#x for pin %u\n",
		    mode, pin);
		return;
	}
	if (!intelgpio_intr_pin(sc, pin, &ip))
		return;

	INTELGPIO_LOCK(sc);
	if (!intelgpio_intr_pin_usable(sc, &ip, pin, true)) {
		INTELGPIO_UNLOCK(sc);
		device_printf(dev, "interrupt pin %u is not host-owned or is locked\n",
		    pin);
		return;
	}
	cfg = bus_read_4(ip.res, ip.pad_off);
	cfg &= ~(INTELGPIO_PADCFG0_PMODE_MASK | INTELGPIO_PADCFG0_GPIORXDIS |
	    INTELGPIO_PADCFG0_GPIROUTNMI | INTELGPIO_PADCFG0_GPIROUTSMI |
	    INTELGPIO_PADCFG0_GPIROUTSCI | INTELGPIO_PADCFG0_GPIROUTIOXAPIC |
	    INTELGPIO_PADCFG0_RXEVCFG_MASK | INTELGPIO_PADCFG0_RXINV);
	cfg |= INTELGPIO_PADCFG0_GPIOTXDIS | event | inv;
	bus_write_4(ip.res, ip.pad_off, cfg);
	INTELGPIO_UNLOCK(sc);
}

static void
intelgpio_pin_enable_intr(device_t dev __unused, uint32_t pin __unused)
{
}

static void
intelgpio_pin_mask_intr(device_t dev, uint32_t pin)
{
	struct intelgpio_softc *sc;
	struct intelgpio_intr_pin ip;
	uint32_t val;

	sc = device_get_softc(dev);

	if (!intelgpio_intr_pin(sc, pin, &ip))
		return;
	INTELGPIO_LOCK(sc);
	val = bus_read_4(ip.res, ip.ie_off);
	bus_write_4(ip.res, ip.ie_off, val & ~ip.bit);
	sc->sc_intr_unmasked[ip.community][ip.group_index] &= ~ip.bit;
	bus_write_4(ip.res, ip.is_off, ip.bit);
	INTELGPIO_UNLOCK(sc);
}

static void
intelgpio_pin_disable_intr(device_t dev, uint32_t pin)
{
	intelgpio_pin_mask_intr(dev, pin);
}

static void
intelgpio_pin_unmask_intr(device_t dev, uint32_t pin)
{
	struct intelgpio_softc *sc;
	struct intelgpio_intr_pin ip;
	uint32_t val;

	sc = device_get_softc(dev);

	if (!intelgpio_intr_pin(sc, pin, &ip))
		return;
	INTELGPIO_LOCK(sc);
	if (sc->sc_intr_ready && intelgpio_intr_pin_usable(sc, &ip, pin, true)) {
		bus_write_4(ip.res, ip.is_off, ip.bit);
		val = bus_read_4(ip.res, ip.ie_off);
		sc->sc_intr_unmasked[ip.community][ip.group_index] |= ip.bit;
		bus_write_4(ip.res, ip.ie_off, val | ip.bit);
	}
	INTELGPIO_UNLOCK(sc);
}

static void
intelgpio_pin_eoi(device_t dev, uint32_t pin)
{
	struct intelgpio_softc *sc;
	struct intelgpio_intr_pin ip;

	sc = device_get_softc(dev);

	if (!intelgpio_intr_pin(sc, pin, &ip))
		return;
	INTELGPIO_LOCK(sc);
	bus_write_4(ip.res, ip.is_off, ip.bit);
	INTELGPIO_UNLOCK(sc);
}

static int
intelgpio_intr_filter(void *arg)
{
	struct intelgpio_softc *sc = arg;
	const struct intelgpio_community *com;
	const struct intelgpio_padgroup *pg;
	struct resource *res;
	bus_size_t is_off, ie_off;
	uint32_t pending;
	device_t busdev;
	int i, j, bit, ret;

	ret = FILTER_STRAY;
	for (i = 0; i < sc->sc_plat->ncommunities; i++) {
		com = &sc->sc_plat->communities[i];
		res = sc->sc_mem_res[i];
		if (!com->has_intr || sc->sc_intr_unmasked[i] == NULL)
			continue;
		for (j = 0; j < com->ngroups; j++) {
			pg = &com->groups[j];
			if (pg->gpio_base < 0 || pg->npads <= 0 || pg->npads > 32)
				continue;
			is_off = com->is_offset + (bus_size_t)pg->reg_num * 4;
			ie_off = com->ie_offset + (bus_size_t)pg->reg_num * 4;
			if (com->is_offset == 0 || com->ie_offset == 0 ||
			    !intelgpio_reg_valid(res, is_off) ||
			    !intelgpio_reg_valid(res, ie_off))
				continue;
			INTELGPIO_LOCK(sc);
			busdev = sc->sc_intr_ready ? sc->sc_busdev : NULL;
			pending = busdev == NULL ? 0 :
			    bus_read_4(res, is_off) & bus_read_4(res, ie_off) &
			    sc->sc_intr_unmasked[i][j];
			INTELGPIO_UNLOCK(sc);

			/* Dispatch can synchronously call back into this driver. */
			while (pending != 0) {
				bit = ffs(pending) - 1;
				pending &= ~(1U << bit);
				gpiobus_handle_intr(busdev, pg->gpio_base + bit);
				ret = FILTER_HANDLED;
			}
		}
	}
	return (ret);
}

int
intelgpio_probe(device_t dev, const struct intelgpio_platform *plat)
{
	int rv;

	if (acpi_disabled("intelgpio"))
		return (ENXIO);

	rv = ACPI_ID_PROBE(device_get_parent(dev), dev, plat->hids, NULL);
	if (rv <= 0)
		device_set_desc(dev, plat->desc);

	return (rv);
}

static int
intelgpio_detach(device_t dev)
{
	struct intelgpio_softc *sc;
	const struct intelgpio_community *com;
	const struct intelgpio_padgroup *pg;
	struct resource *res;
	bus_size_t off;
	uint32_t val;
	int error, i, j;

	sc = device_get_softc(dev);

	/* Stop new dispatches and mask only interrupts enabled by this driver. */
	INTELGPIO_LOCK(sc);
	sc->sc_intr_ready = false;
	for (i = 0; i < sc->sc_plat->ncommunities; i++) {
		com = &sc->sc_plat->communities[i];
		res = sc->sc_mem_res[i];
		if (sc->sc_intr_unmasked[i] == NULL)
			continue;
		for (j = 0; j < com->ngroups; j++) {
			pg = &com->groups[j];
			off = com->ie_offset + (bus_size_t)pg->reg_num * 4;
			if (sc->sc_intr_unmasked[i][j] == 0 ||
			    !intelgpio_reg_valid(res, off))
				continue;
			val = bus_read_4(res, off);
			bus_write_4(res, off, val & ~sc->sc_intr_unmasked[i][j]);
		}
	}
	INTELGPIO_UNLOCK(sc);
	if (sc->sc_irq_cookie != NULL)
		bus_teardown_intr(dev, sc->sc_irq_res, sc->sc_irq_cookie);

	if (sc->sc_busdev != NULL) {
		error = gpiobus_detach_bus(dev);
		if (error != 0) {
			/* Restore delivery if a consumer prevented detachment. */
			if (sc->sc_irq_cookie != NULL &&
			    bus_setup_intr(dev, sc->sc_irq_res,
				INTR_TYPE_MISC | INTR_MPSAFE, intelgpio_intr_filter,
				NULL, sc, &sc->sc_irq_cookie) == 0) {
				INTELGPIO_LOCK(sc);
				sc->sc_intr_ready = true;
				for (i = 0; i < sc->sc_plat->ncommunities; i++) {
					com = &sc->sc_plat->communities[i];
					res = sc->sc_mem_res[i];
					if (sc->sc_intr_unmasked[i] == NULL)
						continue;
					for (j = 0; j < com->ngroups; j++) {
						pg = &com->groups[j];
						off = com->ie_offset +
						    (bus_size_t)pg->reg_num * 4;
						if (sc->sc_intr_unmasked[i][j] == 0 ||
						    !intelgpio_reg_valid(res, off))
							continue;
						val = bus_read_4(res, off);
						bus_write_4(res, off, val |
						    sc->sc_intr_unmasked[i][j]);
					}
				}
				INTELGPIO_UNLOCK(sc);
			} else if (sc->sc_irq_res != NULL) {
				sc->sc_irq_cookie = NULL;
				device_printf(dev, "failed to restore interrupt handler\n");
			}
			return (error);
		}
	}
	sc->sc_irq_cookie = NULL;
	if (sc->sc_irq_res != NULL)
		bus_release_resource(dev, SYS_RES_IRQ, sc->sc_irq_rid,
		    sc->sc_irq_res);

	for (i = 0; i < sc->sc_plat->ncommunities; i++) {
		free(sc->sc_intr_unmasked[i], M_DEVBUF);
		if (sc->sc_mem_res[i] != NULL)
			bus_release_resource(dev, SYS_RES_MEMORY, i,
			    sc->sc_mem_res[i]);
	}

	INTELGPIO_LOCK_DESTROY(sc);
	if (sc->sc_plat != NULL && sc->sc_plat->cleanup != NULL)
		sc->sc_plat->cleanup(sc->sc_plat);
	return (0);
}

int
intelgpio_attach(device_t dev, const struct intelgpio_platform *plat)
{
	struct intelgpio_softc *sc;
	bool have_res, have_intr;
	int error, i, rid;

	sc = device_get_softc(dev);
	sc->sc_dev = dev;
	sc->sc_plat = plat;

	KASSERT(plat->ncommunities <= INTELGPIO_MAX_COMMUNITIES,
	    ("too many communities: %d > %d", plat->ncommunities,
		INTELGPIO_MAX_COMMUNITIES));

	INTELGPIO_LOCK_INIT(sc);

	have_res = false;
	have_intr = false;
	for (i = 0; i < plat->ncommunities; i++) {
		rid = i;
		sc->sc_mem_res[i] = bus_alloc_resource_any(dev, SYS_RES_MEMORY,
		    &rid, RF_ACTIVE);
		if (sc->sc_mem_res[i] == NULL) {
			device_printf(dev,
			    "failed to allocate memory resource "
			    "for community %d\n",
			    i);
			continue;
		}
		if (!intelgpio_reg_valid(sc->sc_mem_res[i], INTELGPIO_PADBAR_REG)) {
			device_printf(dev, "community %d resource is too small\n", i);
			bus_release_resource(dev, SYS_RES_MEMORY, rid,
			    sc->sc_mem_res[i]);
			sc->sc_mem_res[i] = NULL;
			continue;
		}
		sc->sc_padbar[i] = bus_read_4(sc->sc_mem_res[i],
		    INTELGPIO_PADBAR_REG);
		if (plat->communities[i].has_intr &&
		    plat->communities[i].ngroups > 0) {
			sc->sc_revision[i] = bus_read_4(sc->sc_mem_res[i],
			    INTELGPIO_REVID_REG) >> INTELGPIO_REVID_SHIFT;
			sc->sc_intr_unmasked[i] = malloc(
			    sizeof(uint32_t) * plat->communities[i].ngroups,
			    M_DEVBUF, M_WAITOK | M_ZERO);
			have_intr = true;
		}
		have_res = true;
	}

	if (!have_res) {
		device_printf(dev, "no memory resources found\n");
		INTELGPIO_LOCK_DESTROY(sc);
		return (ENXIO);
	}

	if (have_intr) {
		sc->sc_irq_rid = 0;
		sc->sc_irq_res = bus_alloc_resource_any(dev, SYS_RES_IRQ,
		    &sc->sc_irq_rid, RF_ACTIVE | RF_SHAREABLE);
		if (sc->sc_irq_res != NULL) {
			error = bus_setup_intr(dev, sc->sc_irq_res,
			    INTR_TYPE_MISC | INTR_MPSAFE, intelgpio_intr_filter,
			    NULL, sc, &sc->sc_irq_cookie);
			if (error != 0) {
				device_printf(dev, "failed to set up IRQ: %d\n", error);
				bus_release_resource(dev, SYS_RES_IRQ, sc->sc_irq_rid,
				    sc->sc_irq_res);
				sc->sc_irq_res = NULL;
			}
		} else {
			device_printf(dev, "no GPIO interrupt resource\n");
		}
	}

	sc->sc_busdev = gpiobus_add_bus(dev);
	if (sc->sc_busdev == NULL) {
		device_printf(dev, "failed to add gpiobus\n");
		intelgpio_detach(dev);
		return (ENXIO);
	}

	INTELGPIO_LOCK(sc);
	sc->sc_intr_ready = true;
	INTELGPIO_UNLOCK(sc);
	bus_attach_children(dev);

	return (0);
}

static device_method_t intelgpio_methods[] = {
	/* Device interface (probe, attach are in the platform-specific part) */
	DEVMETHOD(device_detach, intelgpio_detach),

	/* GPIO protocol */
	DEVMETHOD(gpio_get_bus, intelgpio_get_bus),
	DEVMETHOD(gpio_pin_max, intelgpio_pin_max),
	DEVMETHOD(gpio_pin_getname, intelgpio_pin_getname),
	DEVMETHOD(gpio_pin_getcaps, intelgpio_pin_getcaps),
	DEVMETHOD(gpio_pin_getflags, intelgpio_pin_getflags),
	DEVMETHOD(gpio_pin_setflags, intelgpio_pin_setflags),
	DEVMETHOD(gpio_pin_get, intelgpio_pin_get),
	DEVMETHOD(gpio_pin_set, intelgpio_pin_set),
	DEVMETHOD(gpio_pin_toggle, intelgpio_pin_toggle),

	/* GPIO interrupt controller interface. */
	DEVMETHOD(gpio_pin_config_intr, intelgpio_pin_config_intr),
	DEVMETHOD(gpio_pin_enable_intr, intelgpio_pin_enable_intr),
	DEVMETHOD(gpio_pin_disable_intr, intelgpio_pin_disable_intr),
	DEVMETHOD(gpio_pin_mask_intr, intelgpio_pin_mask_intr),
	DEVMETHOD(gpio_pin_unmask_intr, intelgpio_pin_unmask_intr),
	DEVMETHOD(gpio_pin_eoi, intelgpio_pin_eoi),

	DEVMETHOD_END
};

DEFINE_CLASS_0(intelgpio, intelgpio_driver, intelgpio_methods,
    sizeof(struct intelgpio_softc));

MODULE_DEPEND(intelgpio, acpi, 1, 1, 1);
MODULE_DEPEND(intelgpio, gpiobus, 1, 1, 1);
MODULE_VERSION(intelgpio, 1);
