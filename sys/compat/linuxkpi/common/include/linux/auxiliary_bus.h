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
	return (0);
}

static inline int
auxiliary_device_uninit(struct auxiliary_device *adev)
{
	return (0);
}


static inline int
auxiliary_device_add(struct auxiliary_device *adev) 
{
        return (0);
}

static inline int
auxiliary_device_delete(struct auxiliary_device *adev)
{
        return (0);
}

#endif
