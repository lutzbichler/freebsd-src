/*
 * Copyright (c) 2026 Lutz Bichler
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef _LINUXKPI_LINUX_PERCPU_H_
#define _LINUXKPI_LINUX_PERCPU_H_

#include <sys/pcpu.h>
#include <sys/sched.h>

struct lkpi_percpu {
    void *_ptr[MAXCPU];
};

extern void *lkpi_alloc_percpu(size_t, size_t);
extern void lkpi_free_percpu(void *);

#define alloc_percpu(type) \
    (type *)lkpi_alloc_percpu(sizeof(type), __alignof__(type))
#define free_percpu(ptr) \
    lkpi_free_percpu(ptr)

#define per_cpu_ptr(ptr, cpu) \
({ \
    struct lkpi_percpu *pcpu__ = (void *)(ptr); \
    (void *)(pcpu__->_ptr[(cpu)]); \
})

#define get_cpu_ptr(ptr) \
({ \
    critical_enter(); \
    per_cpu_ptr(ptr, curcpu); \
})

#define put_cpu_ptr(ptr) \
({ \
    (void)(ptr); \
    critical_exit(); \
})

#endif /* _LINUXKPI_LINUX_PERCPU_H_ */