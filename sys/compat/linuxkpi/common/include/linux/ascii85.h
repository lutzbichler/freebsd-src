#ifndef _LINUXKPI_ASCII85_H_
#define _LINUXKPI_ASCII85_H_

#include <sys/types.h>
#include <sys/param.h>

#define ASCII85_BUFSZ	6

static inline const char*
ascii85_encode(uint32_t value, char *buffer)
{

	if (value == 0)
		return (NULL);

	return (buffer);
}

#endif /* _LINUXKPI_ASCII85_H_ */
