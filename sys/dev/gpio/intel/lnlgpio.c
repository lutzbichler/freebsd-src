/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2026 Lutz Bichler
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/bus.h>
#include <sys/gpio.h>
#include <sys/kernel.h>
#include <sys/module.h>

#include <contrib/dev/acpica/include/acpi.h>
#include <contrib/dev/acpica/include/accommon.h>

#include <dev/acpica/acpivar.h>

#include "intelgpio.h"

#include "gpio_if.h"
#include "opt_acpi.h"

static char *lnl_hids[] = { "INTC105F", NULL };

typedef uint8_t uuid_t[16];

/* UUID for Device Properties Package (Key-Value Strings) */
static const uuid_t dsd_dp_uuid = {
	0x14, 0xd8, 0xff, 0xda,
	0xba, 0x6e,
	0x8c, 0x4d,
	0x8a, 0x91,	0xbc, 0x9b, 0xbf, 0x4a, 0xa3, 0x01
};

/* UUID for Hierarchical Data Extension (Nested Data Nodes) */
static const uuid_t dsd_hde_uuid = {
	0xe6, 0xe3, 0xb8, 0xdb,
	0x86, 0x58,
	0xa6, 0x4b,
	0x87, 0x95, 0x13, 0x19, 0xf5, 0x2a, 0x96, 0x6b
};

static int
acpi_get_integer(ACPI_OBJECT *pkg, const char *name, uint32_t *value)
{
	ACPI_OBJECT *key, *val;

	if (pkg->Type != ACPI_TYPE_PACKAGE || pkg->Package.Count != 2)
		return (EINVAL);

	key = &pkg->Package.Elements[0];
	if (key->Type != ACPI_TYPE_STRING || strcmp(key->String.Pointer, name) != 0)
		return (ENOENT);

	val = &pkg->Package.Elements[1];
	if (val->Type != ACPI_TYPE_INTEGER)
		return (EINVAL);

	*value = (uint32_t)val->Integer.Value;

	return (0);
}

static int
acpi_get_string(ACPI_OBJECT *pkg, const char *name, char **value)
{
	ACPI_OBJECT *key, *val;

	if (pkg->Type != ACPI_TYPE_PACKAGE || pkg->Package.Count != 2)
		return (EINVAL);

	key = &pkg->Package.Elements[0];
	if (key->Type != ACPI_TYPE_STRING ||
		strcmp(key->String.Pointer, name) != 0)
		return (ENOENT);

	val = &pkg->Package.Elements[1];
	if (val->Type != ACPI_TYPE_STRING)
		return (EINVAL);

	*value = val->String.Pointer;

	return (0);
}

static int
lnl_gpio_parse_group(ACPI_HANDLE handle, char *pg_name,
    struct intelgpio_padgroup *pg, int *gpio_base, uint32_t stride)
{
	ACPI_BUFFER buf = { ACPI_ALLOCATE_BUFFER, NULL };
	ACPI_OBJECT *obj, *uuid, *pkg;
	ACPI_STATUS status;
	
	char *name;
	uint32_t offset, pad_count;

	bool has_offset, has_pad_count, has_name;

	status = AcpiEvaluateObject(handle, pg_name, NULL, &buf);
	if (ACPI_FAILURE(status)) {
		printf("%s: Cannot find device object for group %s\n",
		    __func__, pg_name);
		return (ENXIO);
	}

	obj = buf.Pointer;
	if (obj->Type != ACPI_TYPE_PACKAGE || obj->Package.Count < 2) {
		AcpiOsFree(obj);
		printf("%s: incomplete group package for group %s\n",
		    __func__, pg_name);
		return (ENXIO);
	}

	has_offset = false;
	has_pad_count = false;
	has_name = false;

	for (uint32_t i = 0; i < obj->Package.Count - 1; i += 2) {
		uuid = &obj->Package.Elements[i];
		pkg = &obj->Package.Elements[i + 1];

		if (uuid->Type == ACPI_TYPE_BUFFER &&
		    uuid->Buffer.Length == sizeof(uuid_t) &&
		    memcmp(dsd_dp_uuid, uuid->Buffer.Pointer, sizeof(uuid_t)) == 0 &&
		    pkg->Type == ACPI_TYPE_PACKAGE) {

			for (uint32_t j = 0; j < pkg->Package.Count; j++) {
				if (acpi_get_integer(&pkg->Package.Elements[j],
				    "intc-gpio-group-offset", &offset) == 0)
					has_offset = true; 
				else if (acpi_get_integer(&pkg->Package.Elements[j],
				    "intc-gpio-pad-count", &pad_count) == 0)
					has_pad_count = true;
				else if (acpi_get_string(&pkg->Package.Elements[j],
				    "intc-gpio-group-name", &name) == 0)
					has_name = true;
			}
		}
	}

	if (!has_offset || !has_pad_count || !has_name || (offset % stride) != 0) {
		AcpiOsFree(obj);
		return (ENXIO);
	}

	pg->first_pad = offset / stride;
	pg->npads = pad_count;
	pg->name = strdup(name, M_DEVBUF);
	pg->gpio_base = *gpio_base;

	AcpiOsFree(obj);
	*gpio_base += pg->npads;

	return (0);
}

static void
lnl_gpio_cleanup(const struct intelgpio_platform *plat)
{
	struct intelgpio_community *com;
	struct intelgpio_padgroup *pgs;
	int i, j;

	if (plat == NULL)
		return;

	com = __DECONST(struct intelgpio_community *, plat->communities);
	if (com != NULL) {
		for (i = 0; i < plat->ncommunities; i++) {
			pgs = __DECONST(struct intelgpio_padgroup *,
			    com[i].groups);
			if (pgs != NULL) {
				for (j = 0; j < com[i].ngroups; j++)
					free(__DECONST(char *, pgs[j].name), M_DEVBUF);
				free(pgs, M_DEVBUF);
			}
		}
		free(com, M_DEVBUF);
	}
	free(__DECONST(struct intelgpio_platform *, plat), M_DEVBUF);
}

static int
lnl_gpio_parse_config(device_t dev, struct intelgpio_platform *plat)
{
	ACPI_HANDLE handle;
	ACPI_BUFFER buf = { ACPI_ALLOCATE_BUFFER, NULL };
	ACPI_STATUS status;
	ACPI_OBJECT *dsd, *uuid, *pkg;

	struct intelgpio_community *com;
	struct intelgpio_padgroup *pgs;
	int npgs;

	uint32_t i, j, k;
	uint32_t pg_count, com_stride;
	uint32_t gpio_base;
	uint32_t padown_offset, padcfglock_offset, hostown_offset, ie_offset, is_offset;
	uint32_t reg_num, padown_num;

	char subprops[40];
	char *pg_name;

	int rv;

	handle = acpi_get_handle(dev);

	status = AcpiEvaluateObject(handle, "_DSD", NULL, &buf);
	if (ACPI_FAILURE(status)) {
		device_printf(dev, "%s: no _DSD\n", __func__);
		return (ENXIO);
	}

	dsd = buf.Pointer;
	if (dsd->Type != ACPI_TYPE_PACKAGE || dsd->Package.Count < 2) {
		AcpiOsFree(dsd);
		return (ENXIO);
	}

	/* Extract the community data */
	pg_count = 0;
	com_stride = INTELGPIO_PAD_SIZE;

	padown_offset = 0;
	padcfglock_offset = 0;
	hostown_offset = 0;
	ie_offset = 0;
	is_offset = 0;

	/* Community data */
	for (i = 0; i < dsd->Package.Count - 1; i += 2) {
		uuid = &dsd->Package.Elements[i];
		pkg = &dsd->Package.Elements[i+1];

		if (uuid->Type == ACPI_TYPE_BUFFER &&
		    uuid->Buffer.Length == sizeof(uuid_t) &&
		    memcmp(dsd_dp_uuid, uuid->Buffer.Pointer, sizeof(uuid_t)) == 0 &&
		    pkg->Type == ACPI_TYPE_PACKAGE) {

			for (j = 0; j < pkg->Package.Count; j++) {
				acpi_get_integer(&pkg->Package.Elements[j],
				    "intc-gpio-group-count", &pg_count);
				acpi_get_integer(&pkg->Package.Elements[j],
				    "intc-gpio-group-stride", &com_stride);
				acpi_get_integer(&pkg->Package.Elements[j],
				    "intc-gpio-pad-ownership-offset", &padown_offset);
				acpi_get_integer(&pkg->Package.Elements[j],
				    "intc-gpio-pad-configuration-lock-offset",
				    &padcfglock_offset);
				acpi_get_integer(&pkg->Package.Elements[j],
				    "intc-gpio-host-software-pad-ownership-offset",
				    &hostown_offset);
				acpi_get_integer(&pkg->Package.Elements[j],
				    "intc-gpio-gpi-interrupt-status-offset", &is_offset);
				acpi_get_integer(&pkg->Package.Elements[j],
				    "intc-gpio-gpi-interrupt-enable-offset", &ie_offset);
			}
		}
	}

	if (pg_count == 0 || com_stride == 0) {
		AcpiOsFree(dsd);
		device_printf(dev, "%s: invalid group count (%u) or stride (%u)\n",
		    __func__, pg_count, com_stride);
        	return (ENXIO);
	}

	com = malloc(sizeof(*com), M_DEVBUF, M_WAITOK|M_ZERO);
	com->has_intr = true;
	com->ie_offset = ie_offset;
	com->is_offset = is_offset;
	com->padown_offset = padown_offset;
	com->padcfglock_offset = padcfglock_offset;
	com->hostown_offset = hostown_offset;

	pgs = malloc(sizeof(*pgs) * pg_count, M_DEVBUF, M_WAITOK|M_ZERO);

	/* Hierarchical Data Extension */
	reg_num = 0;
	padown_num = 0;
	gpio_base = 0;
	npgs = 0;

	for (i = 0; i < dsd->Package.Count - 1; i += 2) {
		uuid = &dsd->Package.Elements[i];
		pkg = &dsd->Package.Elements[i+1];

		if (uuid->Type == ACPI_TYPE_BUFFER &&
		    uuid->Buffer.Length == sizeof(uuid_t) &&
		    memcmp(dsd_hde_uuid, uuid->Buffer.Pointer, sizeof(uuid_t)) == 0 &&
		    pkg->Type == ACPI_TYPE_PACKAGE) {

			for (j = 0; j < pg_count; j++) {
				snprintf(subprops, sizeof(subprops),
				    "intc-gpio-group-%d-subproperties", j);
				pg_name = NULL;

				for (k = 0; k < pkg->Package.Count; k++) {
					acpi_get_string(&pkg->Package.Elements[k], subprops,
                        &pg_name);
					if (pg_name != NULL) {
						rv = lnl_gpio_parse_group(handle, pg_name,
						    &pgs[npgs], &gpio_base, com_stride);
						if (rv == 0) {
							struct intelgpio_padgroup *pg = &pgs[npgs];
							pg->reg_num = reg_num;
							pg->padown_num = padown_num;
							reg_num += howmany(pg->npads, 32);
							padown_num += howmany(pg->npads, 8);
							npgs++;
						} else
							device_printf(dev, "%s: Error parsing %s\n",
						  	    __func__, pg_name);

						break;
					}
				}
			}
		}
	}

	AcpiOsFree(dsd);

	com->groups = pgs;
	com->ngroups = npgs;

	plat->communities = com;
	plat->ncommunities = 1;
	plat->cleanup = lnl_gpio_cleanup;
    
	return (npgs == 0 ? ENXIO : 0);
}

static const struct intelgpio_platform *
lnl_gpio_create_platform(device_t dev)
{
	struct intelgpio_platform *plat;
	int rv;

	plat = malloc(sizeof(*plat), M_DEVBUF, M_WAITOK|M_ZERO);

	rv = lnl_gpio_parse_config(dev, plat);
	if (rv) {
		lnl_gpio_cleanup(plat);
		return (NULL);
	}

	plat->hids = lnl_hids;
	plat->desc = "Intel Platform GPIO Controller";

	return (plat);
}

/* To probe only! */
static const struct intelgpio_platform lnl_gpio_probe_plat = {
	.hids = lnl_hids,
};

static int
lnl_gpio_probe(device_t dev)
{
	return (intelgpio_probe(dev, &lnl_gpio_probe_plat));
}

static int
lnl_gpio_attach(device_t dev)
{
	const struct intelgpio_platform *plat;
	int rv;

	plat = lnl_gpio_create_platform(dev);
	if (!plat) {
		device_printf(dev, "%s: no platform\n", __func__);
		return (ENXIO);
	}

	rv = intelgpio_attach(dev, plat);
	if (rv)
		lnl_gpio_cleanup(plat);

	return (rv);
}

static device_method_t lnl_methods[] = {
	DEVMETHOD(device_probe, lnl_gpio_probe),
	DEVMETHOD(device_attach, lnl_gpio_attach),

	DEVMETHOD_END
};

DEFINE_CLASS_1(gpio, lnl_driver, lnl_methods,
    sizeof(struct intelgpio_softc), intelgpio_driver);

DRIVER_MODULE(lnlgpio, acpi, lnl_driver, NULL, NULL);
MODULE_DEPEND(lnlgpio, intelgpio, 1, 1, 1);
MODULE_VERSION(lnlgpio, 1);
ACPI_PNP_INFO(lnl_hids);
