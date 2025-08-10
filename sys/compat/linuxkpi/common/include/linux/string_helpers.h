/*
 * Copyright (c) 2025 The FreeBSD Foundation
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef	_LINUXKPI_LINUX_STRING_HELPERS_H_
#define	_LINUXKPI_LINUX_STRING_HELPERS_H_

#include <linux/string_choices.h>

enum string_size_units {
        STRING_UNITS_10,
        STRING_UNITS_2,
};

static inline int
string_get_size(uint64_t size, uint64_t blk_size, enum string_size_units units,
		char* buf, int len)
{
	buf[0] = '\0';
	return (-1);
}

#endif
