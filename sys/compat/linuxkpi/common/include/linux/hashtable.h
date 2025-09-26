/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2022 NVIDIA corporation & affiliates.
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

#ifndef _LINUXKPI_LINUX_HASHTABLE_H
#define	_LINUXKPI_LINUX_HASHTABLE_H

#include <sys/param.h>
#include <sys/systm.h>

#include <linux/hash.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/log2.h>
#include <linux/rcupdate.h>
#include <linux/rculist.h>

#include <ck_queue.h>

#define	DEFINE_HASHTABLE(name, bits) \
	struct hlist_head name[1UL << (bits)]

#define	DEFINE_READ_MOSTLY_HASHTABLE(name, bits) \
	struct hlist_head name[1UL << (bits)] __read_mostly

#define	DECLARE_HASHTABLE(name, bits) \
	struct hlist_head name[1UL << (bits)]

#define	HASH_SIZE(name) ARRAY_SIZE(name)
#define	HASH_BITS(name) ilog2(HASH_SIZE(name))

#define	hash_min(...) \
	hash_long(__VA_ARGS__)

static inline void
__hash_init(struct hlist_head *ht, unsigned long size)
{
	unsigned long x;

	for (x = 0; x != size; x++)
		INIT_HLIST_HEAD(&ht[x]);
}

#define	hash_init(ht) \
	__hash_init(ht, HASH_SIZE(ht))

#define	hash_add(...) \
	hash_add_rcu(__VA_ARGS__)

static inline void
__hash_node_type_assert(struct hlist_node *node)
{
	/*
	 * Unfortunately Linux doesn't have an own type for the hash
	 * table node entries. The purpose of this function is simply
	 * to check the type of the passed argument.
	 */
	CTASSERT(sizeof(struct hlist_node) == sizeof(*node));
}

#define	hash_add_rcu(ht, node, key) do {				\
	struct hlist_head *__head = &(ht)[hash_min(key, HASH_BITS(ht))]; \
	__hash_node_type_assert(node); \
	hlist_add_head((node), __head); \
} while (0)

static inline bool
hash_hashed(struct hlist_node *node)
{
	return ((node)->pprev != NULL);
}

static inline bool
__hash_empty(struct hlist_head *ht, unsigned long size)
{
	unsigned long x;

	for (x = 0; x != size; x++) {
		if (!hlist_empty(&ht[x]))
			return (false);
	}
	return (true);
}

#define	hash_empty(ht) \
	__hash_empty(ht, HASH_SIZE(ht))

#define	hash_del(...) \
	hash_del_rcu(__VA_ARGS__)

static inline void
hash_del_rcu(struct hlist_node *node)
{
	hlist_del_init(node);
}

#define	__hash_first(ht, type, member) ({ \
	const struct hlist_node *__first = (ht)->first; \
	__hash_node_type_assert(&((type *)0)->member); \
	(__first != NULL ? container_of((const void *)__first, type, member) : NULL); \
})

#define	__hash_next(obj, type, member) ({ \
	const struct hlist_node *__next = &((obj)->member)->next; \
	__hash_node_type_assert(&(obj)->member); \
	(__next != NULL ? container_of((const void *)__next, type, member) : NULL); \
})

#define	hash_for_each(...) \
	hash_for_each_rcu(__VA_ARGS__)

#define	hash_for_each_rcu(name, bkt, obj, member) \
	for ((bkt) = 0, (obj) = NULL; (obj) == NULL && \
	       (bkt) != HASH_SIZE(name); (bkt)++) \
				hlist_for_each_entry(obj, &name[bkt], member)

#define	hash_for_each_safe(name, bkt, tmp, obj, member) \
	for ((bkt) = 0, (obj) = NULL; (obj) == NULL && \
	       (bkt) != HASH_SIZE(name); (bkt)++) \
                hlist_for_each_entry_safe(obj, tmp, &name[bkt], member)

#define	hash_for_each_possible(...) \
	hash_for_each_possible_rcu(__VA_ARGS__)

#define	hash_for_each_possible_rcu_notrace(...) \
	hash_for_each_possible_rcu(__VA_ARGS__)

#define	hash_for_each_possible_rcu(name, obj, member, key) \
	hlist_for_each_entry(obj, &name[hash_min(key, HASH_BITS(name))], member)

#define	hash_for_each_possible_safe(name, obj, tmp, member, key) \
	hlist_for_each_entry_safe(obj, tmp, &name[hash_min(key, HASH_BITS(name))], member)

#endif					/* _LINUXKPI_LINUX_HASHTABLE_H */
