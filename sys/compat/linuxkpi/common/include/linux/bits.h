/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2016 Kip Macy
 * Copyright (c) 2018 Johannes Lundberg
 */

#ifndef _LINUXKPI_LINUX_BITS_H_
#define	_LINUXKPI_LINUX_BITS_H_

#include <linux/bitops.h>

#define	__GENMASK(h, l)		(((~0UL) >> (BITS_PER_LONG - (h) - 1)) & ((~0UL) << (l)))
#define	__GENMASK_ULL(h, l)	(((~0ULL) >> (BITS_PER_LONG_LONG - (h) - 1)) & ((~0ULL) << (l)))

#define GENMASK(h, l)       __GENMASK(h, l)
#define GENMASK_ULL(h, l)   __GENMASK_ULL(h, l)

#endif /* _LINUXKPI_LINUX_BITS_H_ */
