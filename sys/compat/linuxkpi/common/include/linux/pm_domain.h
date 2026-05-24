/*
 * Copyright (c) 2026 Lutz Bichler
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef _LINUXKPI_LINUX_PM_DOMAIN_H_
#define _LINUXKPI_LINUX_PM_DOMAIN_H_

#include <linux/kernel.h>

struct generic_pm_domain {
    const char *name;

    int (*power_on)(struct generic_pm_domain *);
    int (*power_off)(struct generic_pm_domain *);

    int (*set_performance_state)(struct generic_pm_domain *, unsigned int);
};

struct dev_power_governor {
};

static inline int
pm_genpd_init(struct generic_pm_domain *genpd, struct dev_power_governor *gov,
        bool is_off) {

    pr_debug("TODO: %s\n", __func__);
    return (-ENOTSUPP);
}

#endif /* _LINUXKPI_LINUX_PM_DOMAIN_H_ */
