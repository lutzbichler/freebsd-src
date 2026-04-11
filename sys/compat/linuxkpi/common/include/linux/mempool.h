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

#ifndef _LINUXKPI_MEMPOOL_H_
#define _LINUXKPI_MEMPOOL_H_

#include <linux/spinlock.h>
#include <linux/types.h>

typedef void* (mempool_alloc_t)(gfp_t mask, void *data);
typedef void  (mempool_free_t)(void *element, void *data);

struct mempool {
    spinlock_t lock;
    int cur_nr;
    int min_nr;
    void **elements;

    mempool_alloc_t *alloc_fn;
    mempool_free_t *free_fn;
    void *data;
};

typedef struct mempool mempool_t;

extern mempool_t * linux_mempool_create(int min_nr, mempool_alloc_t alloc_fn, 
                mempool_free_t free_fn, void *data);
extern int linux_mempool_init(mempool_t *pool, int min_nr,
                mempool_alloc_t alloc_fn, mempool_free_t free_fn, void *data);
extern void linux_mempool_exit(mempool_t *pool);
extern void linux_mempool_destroy(mempool_t *pool);

extern mempool_t * linux_mempool_create_kmalloc_pool(int min_nr, size_t size);
extern int linux_mempool_init_kmalloc_pool(mempool_t *pool, int min_nr, size_t size);

extern bool linux_mempool_initialized(mempool_t *pool);

extern void * linux_mempool_alloc(mempool_t *pool, gfp_t flags);
extern void * linux_mempool_alloc_preallocated(mempool_t *pool);
extern void linux_mempool_free(void *element, mempool_t *pool);

#define mempool_create(min_nr, alloc_fn, free_fn, data) \
    linux_mempool_create(min_nr, alloc_fn, free_fn, data)

#define mempool_init(pool, min_nr, alloc_fn, free_fn, data) \
    linux_mempool_init(pool, min_nr, alloc_fn, free_fn, data)

#define mempool_exit(pool) \
    linux_mempool_exit(pool)

#define mempool_destroy(pool) \
    linux_mempool_destroy(pool)

#define mempool_create_kmalloc_pool(min_nr, size) \
    linux_mempool_create_kmalloc_pool(min_nr, size)

#define mempool_init_kmalloc_pool(pool, min_nr, size) \
    linux_mempool_init_kmalloc_pool(pool, min_nr, size)

#define mempool_initialized(pool) \
    linux_mempool_initialized(pool)

#define mempool_alloc(pool, flags) \
    linux_mempool_alloc(pool, flags)

#define mempool_alloc_preallocated(pool) \
    linux_mempool_alloc_preallocated(pool)

#define mempool_free(element, pool) \
    linux_mempool_free(element, pool)

#endif
