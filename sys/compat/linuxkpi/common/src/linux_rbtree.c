/*-
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

#include <linux/rbtree_augmented.h>

void
lkpi_rb_insert_augmented(struct rb_node *node, struct rb_root *root,
    const struct rb_augment_callbacks *callbacks)
{
    struct rb_node *parent;

    parent = rb_parent(node);

    if (callbacks != NULL && callbacks->propagate != NULL)
        callbacks->propagate(node, NULL);

    rb_insert_color(node, root);

    if (parent != NULL && callbacks != NULL && callbacks->propagate != NULL)
        callbacks->propagate(parent, NULL);
}

void
lkpi_rb_erase_augmented(struct rb_node *node, struct rb_root *root,
    const struct rb_augment_callbacks *callbacks)
{
    struct rb_node *parent;
    struct rb_node *succ;

    parent = rb_parent(node);
    succ = NULL;

    if (RB_LEFT(node, __entry) && RB_RIGHT(node, __entry)) {
        succ = RB_RIGHT(node, __entry);
        while (RB_LEFT(succ, __entry))
            succ = RB_LEFT(succ, __entry);

        parent = rb_parent(succ);
        if (parent == node)
            parent = succ;

        if (callbacks != NULL && callbacks->copy != NULL)
            callbacks->copy(node, succ);
    }

    rb_erase(node, root);

    if (parent != NULL && callbacks != NULL && callbacks->propagate != NULL)
        callbacks->propagate(parent, NULL);
}
