/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2024, Lutz Bichler <lutz.bichler@gmail.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
#ifndef _LINUXKPI_SEQ_BUF_H_
#define _LINUXKPI_SEQ_BUF_H_

#include <linux/stdarg.h>
#include <linux/types.h>

struct seq_buf {
	char	*buf;
	size_t	len;
	size_t	cur;
}; 

#define DECLARE_SEQ_BUF(n, l)  	\
	struct seq_buf n = {		\
		.buf = (char[l]) { 0 },	\
		.len = l,				\
		.cur = 0,				\
	}

static inline const char *
seq_buf_str(const struct seq_buf *s)
{

	return (s->buf);
}

static inline int
seq_buf_has_overflowed(const struct seq_buf *s)
{

	return (s->cur > s->len);
}

void lkpi_seq_buf_vprintf(struct seq_buf *s, const char *fmt, va_list args);
void lkpi_seq_buf_printf(struct seq_buf *s, const char *fmt, ...);

#define	seq_buf_vprintf(...)	lkpi_seq_buf_vprintf(__VA_ARGS__)
#define	seq_buf_printf(...)		lkpi_seq_buf_printf(__VA_ARGS__)

#endif /* _LINUXKPI_SEQ_BUF_H_ */

