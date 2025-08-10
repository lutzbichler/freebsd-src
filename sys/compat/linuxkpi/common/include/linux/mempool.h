#ifndef _LINUXKPI_MEMPOOL_H_
#define _LINUXKPI_MEMPOOL_H_

typedef struct {
} mempool_t;

static inline bool
mempool_initialized(mempool_t *pool)
{

    return (false);
}

static inline void *
mempool_alloc(mempool_t *pool, gfp_t flags)
{

    return (NULL);
}

static inline void
mempool_free(void *mem, mempool_t *pool)
{

}

static inline void
mempool_exit(mempool_t *pool)
{

}

static inline int
mempool_init_kmalloc_pool(mempool_t *pool, int nr, size_t size)
{

    return (0);
}

#endif /* _LINUXKPI_MEMPOOL_H_ */
