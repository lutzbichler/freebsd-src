/*
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef LINUXKPI_PAGE_WALK_H_
#define LINUXKPI_PAGE_WALK_H_

#include <linux/mm.h>

enum page_walk_action {
       ACTION_AGAIN,
       ACTION_CONTINUE,
       ACTION_SUBTREE,
};

#endif
