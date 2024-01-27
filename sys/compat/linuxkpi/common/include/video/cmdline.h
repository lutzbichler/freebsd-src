/* Public domain */

#ifndef LINUXKPI_VIDEO_CMDLINE_H_
#define LINUXKPI_VIDEO_CMDLINE_H_

#define video_get_options(cmdline) \
	linux_video_get_options(cmdline)

char *linux_video_get_options(const char *connector_name);

#endif /* LINUXKPI_VIDEO_CMDLINE_H_ */

