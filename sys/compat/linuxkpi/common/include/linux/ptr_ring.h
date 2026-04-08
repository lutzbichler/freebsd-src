/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2026 Lutz Bichler (Lutz.Bichler@gmail.com)
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice unmodified, this list of conditions, and the following
 *    disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef LINUXKPI_PTR_RING_H_
#define LINUXKPI_PTR_RING_H_

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

#endif /* LINUXKPI_PTR_RING_H_ */

