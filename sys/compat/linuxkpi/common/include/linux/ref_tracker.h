/* Public domain */

#ifndef LINUXKPI_REF_TRACKER_H_
#define LINUXKPI_REF_TRACKER_H_

struct ref_tracker {
};

struct ref_tracker_dir {
};

static inline int
ref_tracker_alloc(struct ref_tracker_dir *dir, struct ref_tracker **tracker,  gfp_t flags)
{

	return (0);
}

static inline int
ref_tracker_free(struct ref_tracker_dir *dir, struct ref_tracker **tracker)
{

	return (0);
}

static inline size_t
ref_tracker_dir_snprint(struct ref_tracker_dir *dir, char *buf, size_t len)
{

	return (0);
}

#endif /* LINUXKPI_REF_TRACKER_H_ */

