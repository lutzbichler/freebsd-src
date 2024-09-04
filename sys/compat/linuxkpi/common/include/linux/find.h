/*-
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef _LINUXKPI_FIND_H
#define _LINUXKPI_FIND_H

#include <linux/bitops.h>

#define for_each_clear_bitrange(start, end, addr, size) \
        for (start = 0; \
             start = find_next_zero_bit(addr, size, start), \
             end = find_next_bit(addr, size, start + 1), \
             start < size; \
             start = end + 1)

#define for_each_set_bitrange(start, end, addr, size) \
        for (start = 0; \
             start = find_next_bit(addr, size, start), \
             end = find_next_bit(addr, size, start + 1), \
             start < size; \
             start = end + 1)

#define for_each_or_bit(start, left, right, size) \
        for (start = 0; \
             start = find_next_or_bit(left, right, size, start), \
                    start < size; \
             start++)

#endif /* _LINUXKPI_FIND_H */
