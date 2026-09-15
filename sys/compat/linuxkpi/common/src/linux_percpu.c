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

#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/malloc.h>
#include <sys/proc.h>
#include <sys/smp.h>

#include <linux/percpu.h>

static MALLOC_DEFINE(M_LKPI_PCPU, "lkpipcpu", "LinuxKPI percpu compat");

 void *
 lkpi_alloc_percpu(size_t size, size_t align)
 {
    struct lkpi_percpu *pcpu;
    int i;

    pcpu = malloc(sizeof(*pcpu), M_LKPI_PCPU, M_WAITOK | M_ZERO);

    CPU_FOREACH(i) {
        if (align > 0) {
            pcpu->_ptr[i] = malloc(roundup(size, align), M_LKPI_PCPU, M_WAITOK | M_ZERO);
        } else {
            pcpu->_ptr[i] = malloc(size, M_LKPI_PCPU, M_WAITOK | M_ZERO);
        }
    }

    return  ((void *)pcpu);
 }

 void
 lkpi_free_percpu(void *ptr)
 {
    struct lkpi_percpu *pcpu;
    int i;

    if (ptr == NULL)
        return;

    pcpu = ptr;

    CPU_FOREACH(i) {
        free(pcpu->_ptr[i], M_LKPI_PCPU);
    }

    free(pcpu, M_LKPI_PCPU);
 }