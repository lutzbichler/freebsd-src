/* Public domain */

#ifndef _LINUXKPI_KMSG_DUMP_H_
#define _LINUXKPI_KMSG_DUMP_H_

struct kmsg_dumper;

enum kmsg_dump_reason {
	KMSG_DUMP_PANIC
};

typedef void (*kmsg_dump_panic)(struct kmsg_dumper *, enum kmsg_dump_reason);

struct kmsg_dumper {
	kmsg_dump_panic		dump;
	enum kmsg_dump_reason	max_reason;
};

static inline int
kmsg_dump_register(const struct kmsg_dumper *d)
{
	return (0);
}

static inline void
kmsg_dump_unregister(const struct kmsg_dumper *d)
{
}

#endif /* _LINUXKPI_KMSG_DUMP_ */

