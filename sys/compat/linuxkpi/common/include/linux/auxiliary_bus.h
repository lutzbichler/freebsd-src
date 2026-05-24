/*
 * Copyright (c) 2025 Lutz Bichler
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef _LINUXKPI_AUXILIARY_BUS_H_
#define _LINUXKPI_AUXILIARY_BUS_H_

#include <linux/device.h>
#include <linux/kernel.h>	/* pr_debug */

struct auxiliary_device {
	struct device dev;
	const char *name;
	uint32_t id;
};

static inline int
auxiliary_device_init(struct auxiliary_device *adev)
{

	pr_debug("%s: TODO\n", __func__);
	return (0);
}

static inline int
auxiliary_device_uninit(struct auxiliary_device *adev)
{

	pr_debug("%s: TODO\n", __func__);
	return (0);
}

static inline int
auxiliary_device_add(struct auxiliary_device *adev)
{

	pr_debug("%s: TODO\n", __func__);
	return (0);
}

static inline int
auxiliary_device_delete(struct auxiliary_device *adev)
{

	pr_debug("%s: TODO\n", __func__);
	return (0);
}

#endif /* _LINUXKPI_AUXILIARY_BUS_H_ */
