/* Public domain */

#ifndef _LINUXKPI_FONT_H_
#define _LINUXKPI_FONT_H_

struct font_desc {
	unsigned int width;
	unsigned int height;
	const void *data;
};

#endif /* _LINUXKPI_FONT_H_ */
