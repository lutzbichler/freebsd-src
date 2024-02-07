/* Public domain */

#ifndef LINUXKPI_MATH_H_
#define LINUXKPI_MATH_H_

#define abs_diff(a, b) ({			\
	typeof(a) __a = (a);			\
	typeof(b) __b = (b);			\
	(void) (&__a == &__b);			\
	__a > __b ? (__a - __b) : (__b - __a); })

#endif /* LINUXKPI_MATH_H_ */

