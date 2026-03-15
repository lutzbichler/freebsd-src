/*-
 * Copyright (c) 2025 Lutz Bichler (Lutz.Bichler@gmail.com)
 * All rights reserved.
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

#include <linux/mempool.h>
#include <linux/slab.h>

mempool_t *
linux_mempool_create(int min_nr, mempool_alloc_t alloc_fn,
    mempool_free_t free_fn, void *data)
{
    mempool_t *pool = kmalloc(sizeof(mempool_t), 0);
    if (pool)
        linux_mempool_init(pool, min_nr, alloc_fn, free_fn, data);
    return (pool);
}

int
linux_mempool_init(mempool_t *pool, int min_nr, mempool_alloc_t alloc_fn,
    mempool_free_t free_fn, void *data)
{
    int i;

    spin_lock_init(&pool->lock);
    pool->min_nr = min_nr;
    pool->alloc_fn = alloc_fn;
    pool->free_fn = free_fn;
    pool->data = data;

    pool->cur_nr = 0;
    pool->elements = kmalloc_array(min_nr, sizeof(void *), 0);
    if (pool->elements) {
        for (i = 0; i < pool->min_nr; i++)
            pool->elements[i] = alloc_fn(0, pool->data);
    }

    // Error handling?
    return (0);
}

void
linux_mempool_exit(mempool_t *pool)
{
    int i;

    for (i = 0; i < pool->cur_nr; i++)
        pool->free_fn(pool->elements[i], pool->data);

    kfree(pool->elements);
    pool->elements = NULL;
}

void
linux_mempool_destroy(mempool_t *pool)
{
    if (pool == NULL)
        return;

    linux_mempool_exit(pool);
    kfree(pool);
}

static void *
linux_mempool_kmalloc(gfp_t mask, void *data)
{
    size_t size = *(size_t *)data;
    return (kmalloc(size, mask));
}

static void
linux_mempool_kfree(void *element, void *data)
{
    kfree(element);
}

mempool_t *
linux_mempool_create_kmalloc_pool(int min_nr, size_t size)
{
    return (linux_mempool_create(min_nr, linux_mempool_kmalloc,
                linux_mempool_kfree, (void*)(unsigned long)size));
}

int
linux_mempool_init_kmalloc_pool(mempool_t *pool, int min_nr, size_t size)
{
    return (linux_mempool_init(pool, min_nr, linux_mempool_kmalloc,
                linux_mempool_kfree, (void*)(unsigned long)size));
}

bool
linux_mempool_initialized(mempool_t *pool)
{
    return (pool && pool->elements);
}

void *
linux_mempool_alloc(mempool_t *pool, gfp_t flags)
{
    void *res;

    res = pool->alloc_fn(flags, pool->data);
    if (res)
        return res;

    return (linux_mempool_alloc_preallocated(pool));
}

void *
linux_mempool_alloc_preallocated(mempool_t *pool)
{
    void *res;
    
    res = NULL;

    spin_lock(&pool->lock);
    if (pool->cur_nr < pool->min_nr) {
        res = pool->elements[pool->cur_nr];
        pool->cur_nr++;
    }
    spin_unlock(&pool->lock);

    return (res);
}

void
linux_mempool_free(void *element, mempool_t *pool)
{
    bool free_element;
    int i;

    free_element = true;
    i = 0;

    spin_lock(&pool->lock);
    while (pool->elements[i] != element && i < pool->cur_nr) i++;
    if (i < pool->cur_nr) {
        while (i < pool->cur_nr) {
            pool->elements[i] = pool->elements[i+1];
            i++;           
        }
        pool->elements[pool->cur_nr] = element;
        free_element = false;
    }
    spin_unlock(&pool->lock);

    if (free_element)
        pool->free_fn(element, pool->data);
}