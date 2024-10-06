#ifndef LINUXKPI_ARGS_H_
#define LINUXKPI_ARGS_H_

#include <sys/cdefs.h>

#define LKPI_COUNT_ARGS(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _n, X...) _n
#define COUNT_ARGS(X...) LKPI_COUNT_ARGS(, ##X, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

#define CONCATENATE(x, y) __CONCAT(x, y)

#endif /* LINUXKPI_ARGS_H_ */

