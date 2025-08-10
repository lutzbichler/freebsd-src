#ifndef _LINUXKPI_KUNIT_STATIC_STUB_H_
#define _LINUXKPI_KUNIT_STATIC_STUB_H_

#define KUNIT_STATIC_STUB_REDIRECT(args...)

static inline int
kunit_get_current_test(void)
{

	return (0);
}

#endif /* _LINUXKPI_KUNIT_STATIC_STUB_H_ */

