/* Public domain */

#ifndef _LINUXKPI_LINUX_UTIL_MACROS_H_
#define _LINUXKPI_LINUX_UTIL_MACROS_H_

#if defined(LINUXKPI_VERSION) && (LINUXKPI_VERSION > 61400)
#define for_each_if(condition) if (!(condition)) {} else
#endif

#endif /* _LINUXKPI_LINUX_UTIL_MACROS_H_ */

