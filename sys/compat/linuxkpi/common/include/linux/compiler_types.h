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
#ifndef	_LINUXKPI_LINUX_COMPILER_TYPES_H_
#define	_LINUXKPI_LINUX_COMPILER_TYPES_H_

#include <sys/cdefs.h>

#include <compat/linuxkpi/common/include/linux/compiler_attributes.h>

#define __kernel
#define __user
#define __iomem
#define	__percpu
#define	__rcu
#define __chk_user_ptr(x)		((void)0)
#define __chk_io_ptr(x)			((void)0)
#define __acquires(x)
#define __releases(x)
#define __acquire(x)			do { } while (0)
#define __release(x)			do { } while (0)
#define __cond_lock(x,c)		(c)
#define __force
#define __nocast
#define __safe
#define __builtin_warning(x, y...)	(1)

#define	___PASTE(a,b) a##b
#define	__PASTE(a,b) ___PASTE(a,b)

#define __diag_push()
#define __diag_pop()
#define __diag_ignore_all(...)

#define	__same_type(a, b)	__builtin_types_compatible_p(typeof(a), typeof(b))

#endif	/* _LINUXKPI_LINUX_COMPILER_TYPES_H_ */
