/*-
 * Copyright (c) 2010 Isilon Systems, Inc.
 * Copyright (c) 2010 iX Systems, Inc.
 * Copyright (c) 2010 Panasas, Inc.
 * Copyright (c) 2013-2016 Mellanox Technologies, Ltd.
 * Copyright (c) 2015 François Tigeot
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
#ifndef	_LINUXKPI_LINUX_COMPILER_ATTRIBUTES_H_
#define	_LINUXKPI_LINUX_COMPILER_ATTRIBUTES_H_

#include <sys/cdefs.h>

#define	__attribute_const__		__attribute__((__const__))

#ifndef	__deprecated
#define	__deprecated
#endif

#define	fallthrough			/* FALLTHROUGH */ do { } while(0)

#undef __always_inline
#define	__always_inline			inline

#define	__printf(a,b)			__printflike(a,b)

#define	__malloc

#define	noinline			__noinline

#if __has_attribute(__nonstring__)
#define	__nonstring			__attribute__((__nonstring__))
#else
#define	__nonstring
#endif

#define	noinline_for_stack		__noinline

#define	__maybe_unused			__unused
#define	__always_unused			__unused

#define	__must_check			__result_use_check

#define	__weak __weak_symbol

#endif	/* _LINUXKPI_LINUX_COMPILER_ATTRIBUTES_H_ */
