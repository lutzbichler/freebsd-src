#ifndef LINUXKPI_ASM_VIDEO_H_
#define LINUXKPI_ASM_VIDEO_H_

#include <linux/kernel.h>

static inline bool
video_is_primary_device(struct device *dev)
{

	pr_warn("%s is not implemented!\n", __func__);
	return (true);
}

#endif /* LINUXKPI_ASM_VIDEO_H_ */
