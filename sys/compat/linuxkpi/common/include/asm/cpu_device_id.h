/* Public domain */

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
