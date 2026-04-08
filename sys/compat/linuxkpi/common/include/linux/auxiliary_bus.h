/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2025 Lutz Bichler (Lutz.Bichler@gmail.com)
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

#ifndef _LINUXKPI_AUXILIARY_BUS_H_
#define _LINUXKPI_AUXILIARY_BUS_H_

#include <linux/device.h>

struct auxiliary_device {
	struct device dev;
	const char *name;
	uint32_t id;
};

static inline int
auxiliary_device_init(struct auxiliary_device *adev)
{

	pr_warn("TODO: Implement %s\n", __func__);
	return (0);
}

static inline int
auxiliary_device_uninit(struct auxiliary_device *adev)
{

	pr_warn("TODO: Implement %s\n", __func__);
	return (0);
}

static inline int
auxiliary_device_add(struct auxiliary_device *adev)
{

	pr_warn("TODO: Implement %s\n", __func__);
	return (0);
}

static inline int
auxiliary_device_delete(struct auxiliary_device *adev)
{

	pr_warn("TODO: Implement %s\n", __func__);
	return (0);
}

#endif /* _LINUXKPI_AUXILIARY_BUS_H_ */
