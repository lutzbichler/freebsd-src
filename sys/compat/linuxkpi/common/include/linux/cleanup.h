/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2024 The FreeBSD Foundation
 *
 * This software was developed by Björn Zeeb under sponsorship from
 * the FreeBSD Foundation.
 */

#ifndef	_LINUXKPI_LINUX_CLEANUP_H
#define	_LINUXKPI_LINUX_CLEANUP_H

#define	__cleanup(_f)		__attribute__((__cleanup__(_f)))

/*
 * Note: "_T" are special as they are exposed into common code for
 * statements.  Extra care should be taken when changing the code.
 */
#define	DEFINE_GUARD(_n, _dt, _lock, _unlock)				\
									\
    typedef _dt guard_ ## _n ## _t;					\
									\
    static inline _dt							\
    guard_ ## _n ## _create( _dt _T)					\
    {									\
	_dt c;								\
									\
	c = ({ _lock; _T; });						\
	return (c);							\
    }									\
									\
    static inline void							\
    guard_ ## _n ## _destroy(_dt *t)					\
    {									\
	_dt _T;								\
									\
	_T = *t;							\
	if (_T) { _unlock; };						\
    }

/* We need to keep these calls unique. */
#define	guard(_n)							\
    guard_ ## _n ## _t guard_ ## _n ## _ ## __COUNTER__			\
	__cleanup(guard_ ## _n ## _destroy) = guard_ ## _n ## _create

#define DEFINE_FREE(_n, _t, _free)					\
static inline void __free_##_n(void *p) {				\
	_t _T = *(_t *)p;						\
	 _free;								\
}

#define __free(_n)   __cleanup(__free_##_n)

#define LKPI_DEFINE_CLASS(_n, _t, _e, _i, _a...)		\
typedef _t class_##_n##_##t;							\
static inline void class_##_n##_dtor(_t *obj)				\
{									\
    _t _T = *obj; _e;						\
}									\
static inline _t class_##_n##_ctor(_a)				\
{									\
    _t obj = _i;							\
    return obj;								\
}

#define LKPI_CLASS(_n, _v)						\
        class_##_n##_t _v __cleanup(class_##_n##_dtor) = class_##_n##_ctor

#define __scoped_guard(_n, _l, _a)

#define scoped_guard(_n, _a)						\
	__scoped_guard(_n, _l ## __COUNTER__, _a)

#endif	/* _LINUXKPI_LINUX_CLEANUP_H */
