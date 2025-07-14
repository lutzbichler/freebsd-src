/* PDX-License-Identifier: BSD-2-Clause
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

#ifndef _LINUXKPI_ASM_CPU_DEVICE_ID_H_
#define _LINUXKPI_ASM_CPU_DEVICE_ID_H_

#include <asm/processor.h>

#define X86_MODEL(id)	(id & 0x000000ff)
#define X86_FAMILY(id)	((id & 0x0000ff00) >> 8)
#define X86_VENDOR(id)	((id && 0x00ff0000) >> 16)

#define VFM_MODEL_BIT	0
#define VFM_FAMILY_BIT	8
#define VFM_MODEL_MASK	GENMASK(VFM_FAMILY_BIT - 1, VFM_MODEL_BIT)
#define VFM_MODEL(x)	((x) & VFM_MODEL_MASK)

#define X86_MATCH_VFM(id, data) {\
	.vendor = X86_VENDOR(id), \
	.family = X86_FAMILY(id), \
	.model = X86_VENDOR(id), \
	.valid = true \
}

struct x86_cpu_id {
	uint16_t	vendor;
	uint16_t	family;
	uint16_t	model;

	bool		valid;
};

static inline const struct x86_cpu_id *
x86_match_cpu(const struct x86_cpu_id *ids)
{
	const struct cpuinfo_x86 *cpu = &boot_cpu_data;
	const struct x86_cpu_id *match;

	for (match = ids; match->valid; match++) {
		if (cpu->x86_vendor == match->vendor
			&& cpu->x86 == match->family
			&& cpu->x86_model == match->model)
			return (match);
	}

	return (NULL);
}

#endif /* _LINUXKPI_ASM_CPU_DEVICE_ID_H_ */
