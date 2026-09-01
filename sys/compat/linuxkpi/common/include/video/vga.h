/* Public domain. */

#ifndef _LINUXKPI_VIDEO_VGA_H
#define _LINUXKPI_VIDEO_VGA_H

#include <linux/types.h>
#include <linux/io.h>

#define VGA_MIS_W	0x3c2
#define VGA_SEQ_I	0x3c4
#define VGA_SEQ_D	0x3c5
#define VGA_MIS_R	0x3cc

#define VGA_IS0_R	0x3c2 

#define VGA_SR01_SCREEN_OFF	(1 << 5)

#define VGA_FB_PHYS_BASE	0xA0000
#define VGA_FB_PHYS_SIZE	65536

#define VGA_MIS_COLOR           0x1
#define VGA_MIS_ENB_MEM_ACCESS  0x2

#endif
