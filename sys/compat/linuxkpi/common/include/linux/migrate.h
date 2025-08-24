/* Public domain */

#ifndef _LINUXKPI_MIGRATE_H_
#define _LINUXKPI_MIGRATE_H_

#include <linux/page.h>

#define MIGRATE_PFN_VALID       1
#define MIGRATE_PFN_MIGRATE     2
#define MIGRATE_PFN_WRITE       4
#define MIGRATE_PFN_SHIFT       6

static inline struct page *
migrate_pfn_to_page(unsigned long pfn)
{
    if ((pfn & MIGRATE_PFN_VALID) == 0)
        return NULL;

    return pfn_to_page(pfn);
}

#endif /* _LINUXKPI_MIGRATE_H_ */