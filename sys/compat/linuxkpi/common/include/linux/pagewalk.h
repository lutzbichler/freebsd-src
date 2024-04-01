/* Public domain */

#ifndef LINUXKPI_PAGE_WALK_H_
#define LINUXKPI_PAGE_WALK_H_

#include <linux/mm.h>

enum page_walk_action {
	ACTION_AGAIN,
	ACTION_CONTINUE,
	ACTION_SUBTREE,
};

#endif /* LINUXKPI_PAGE_WALK_H_ */

