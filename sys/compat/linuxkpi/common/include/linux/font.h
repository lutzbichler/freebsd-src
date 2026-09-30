/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025-2026 The FreeBSD Foundation
 * Copyright (c) 2025-2026 Jean-Sébastien Pédron <dumbbell@FreeBSD.org>
 *
 * This software was developed by Jean-Sébastien Pédron under sponsorship
 * from the FreeBSD Foundation.
 */

#ifndef _LINUXKPI_LINUX_FONT_H_
#define	_LINUXKPI_LINUX_FONT_H_

#include <linux/math.h>
#include <linux/types.h>

struct font_desc {
	const char *name;
	const u8 *data;
	int idx;
	unsigned int width;
	unsigned int height;
	unsigned int charcount;
	int pref;
};

static inline const struct font_desc *
get_default_font(int xres, int yres, unsigned long *font_w,
    unsigned long *font_h)
{
	return (NULL);
}

static inline const u8 *
font_data_glyph_buf(const u8 *font_data, unsigned int width, unsigned int height,
	unsigned int c)
{
	return (font_data + (c * height) * DIV_ROUND_UP(width, 8));
}

#endif
