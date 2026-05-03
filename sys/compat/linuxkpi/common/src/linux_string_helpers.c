/*
 * Copyright (c) 2026 Lutz Bichler
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <linux/array_size.h>
#include <linux/math64.h>
#include <linux/string_helpers.h>

static inline unsigned
lkpi_string_get_size_helper(uint64_t *size, uint64_t div)
{
    unsigned digit = 0;
    uint64_t scaled = 0;

    for (unsigned i = 0; i < 10; i++) {
        if (scaled >= div - *size) {
            scaled -= div - *size;
            digit++;
        } else {
            scaled += *size;
        }
    }

    *size = scaled;

    return digit;
}

int
linux_string_get_size(uint64_t size, uint64_t blk_size,
	enum string_size_units units, char* buf, int len)
{
    static const char *string_units_10[] = {"B", "kB", "MB", "GB", "TB", "PB",
		                                    "EB", "ZB", "YB"};
    static const char *string_units_2[]  = {"B", "KiB", "MiB", "GiB", "TiB",
		                                    "PiB", "EiB", "ZiB", "YiB"};
    char remaining[8];
    const char *entity;
    uint64_t f, frac, i, n, r, tmp;

    if (size == 0 || blk_size == 0) {
        buf[0] = '\0';
        return (0);
    }

    f = (uint64_t)(units == STRING_UNITS_10 ? 1000 : 1024);

    frac = 0;

    // avoid overflow by limiting size and blk_size to 32-binary_units
    while (size > UINT32_MAX) {
        size /= f;
        frac++;
    }
    while (blk_size > UINT32_MAX) {
        blk_size /= f;
        frac++;
    }

	// Determine whole and remaining
    size *= blk_size;
    while (size > f) {
        r = do_div(size, f);
        frac++;
    }

    n = 0;
    tmp = size;
    while (tmp > 0) {
        n++;
        tmp /= 10;
    }

    for (i = 0; i < 5-n; i++) {
        remaining[i] = '0' + lkpi_string_get_size_helper(&r, f);
    }

    // Entity
    if (frac < ARRAY_SIZE(string_units_2)) {
        if (units == STRING_UNITS_10)
            entity = string_units_10[frac];
        else
            entity = string_units_2[frac];
    } else {
        entity = "unk";
    }

    return (snprintf(buf, len, "%lu.%s%s", size, remaining, entity));
}