/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2026 Lutz Bichler
 *
 */
#ifndef _LINUXKPI_PTR_RING_H
#define _LINUXKPI_PTR_RING_H

#include <sys/buf_ring.h>

#include <linux/errno.h>
#include <linux/gpf.h>
#include <linux/slab.h>
#include <linux/spinlock.h>

struct ptr_ring {
    struct buf_ring *br;
    spinlock_t lock;
};

static inline int
ptr_ring_init(struct ptr_ring *ring, int size, gfp_t gfp)
{
    spin_lock_init(&ring->lock);
    ring->br = buf_ring_alloc(size, M_KMALLOC, gfp, &ring->lock);
    if (ring->br == NULL)
        spin_lock_destroy(&ring->lock);

    return (ring->br != NULL ? 0 : -ENOMEM);
}

typedef void (*ptr_ring_destroy_func_t)(void *);

static inline void
ptr_ring_cleanup(struct ptr_ring *ring, ptr_ring_destroy_func_t _func)
{
    void *data;

    if (ring != NULL) {
        spin_lock(&ring->lock);
        if (ring->br != NULL) {
            if (_func != NULL) {
                while (FALSE == buf_ring_empty(ring->br)) {
                    data = buf_ring_dequeue_sc(ring->br);
                    if (data != NULL)
                        _func(data);
                }
            }
            buf_ring_free(ring->br, M_KMALLOC);
            ring->br = NULL;
        }
        spin_unlock(&ring->lock);

        spin_lock_destroy(&ring->lock);
    }
}

static inline bool
ptr_ring_empty(struct ptr_ring *ring)
{
    return (buf_ring_empty(ring->br));
}

static inline bool
ptr_ring_full(struct ptr_ring *ring)
{
    return (buf_ring_full(ring->br));
}


static inline int
ptr_ring_produce(struct ptr_ring *ring, void *data)
{
    return (buf_ring_enqueue(ring->br, data) == 0 ? 0 : -ENOSPC);
}

static inline void *
ptr_ring_consume(struct ptr_ring *ring)
{
    void *data;

    data = NULL;

    spin_lock(&ring->lock);
    data = buf_ring_dequeue_sc(ring->br);
    spin_unlock(&ring->lock);

    return (data);
}

#endif /* _LINUXKPI_PTR_RING_H */