#ifndef	_LINUXKPI_LINUX_RBTREE_AUGMENTED_H_
#define	_LINUXKPI_LINUX_RBTREE_AUGMENTED_H_

#include <linux/rbtree.h>

struct rb_augment_callbacks {
    void (*propagate)(struct rb_node *, struct rb_node *);
    void (*copy)(struct rb_node *, struct rb_node *);
    void (*rotate)(struct rb_node *, struct rb_node *);
};

#define RB_DECLARE_CALLBACKS(__STATIC, __NAME, __STRUCT, __FIELD,	\
		  	     __AUGMENTED,  __COMPUTE)			\
									\
static inline void							\
__NAME##_propagate(struct rb_node *node, struct rb_node *root)		\
{									\
	while (node != root) {						\
		__STRUCT *n = rb_entry(node, __STRUCT, __FIELD);	\
		if (__COMPUTE(n, true))					\
			break;						\
		node = rb_parent(&n->__FIELD);				\
	}								\
}									\
									\
static inline void							\
__NAME##_copy(struct rb_node *tgt, struct rb_node *src)			\
{									\
	__STRUCT *s = rb_entry(src, __STRUCT, __FIELD);			\
	__STRUCT *t = rb_entry(tgt, __STRUCT, __FIELD);			\
	t->__AUGMENTED = s->__AUGMENTED;				\
}									\
									\
static void								\
__NAME##_rotate(struct rb_node *tgt, struct rb_node *src)		\
{									\
	__STRUCT *s = rb_entry(src, __STRUCT, __FIELD);			\
	__STRUCT *t = rb_entry(tgt, __STRUCT, __FIELD);			\
	t->__AUGMENTED = s->__AUGMENTED;				\
	__COMPUTE(s, false);						\
}									\
									\
__STATIC const struct rb_augment_callbacks __NAME = {			\
	.propagate = __NAME##_propagate,				\
	.copy = __NAME##_copy,						\
	.rotate = __NAME##_rotate					\
};

#define RB_DECLARE_CALLBACKS_MAX(__STATIC, __NAME, __STRUCT, __FIELD,	\
				 __TYPE, __AUGMENTED, __COMPUTE)	\
									\
static inline bool __NAME##_compute_max(__STRUCT *node, bool exit)	\
{									\
	__STRUCT *c;							\
	__TYPE m = __COMPUTE(node);					\
	if (node->__FIELD.rb_left) {					\
		c = rb_entry(node->__FIELD.rb_left, __STRUCT, __FIELD);	\
		if (c->__AUGMENTED > m)					\
			m = c->__AUGMENTED;				\
	}								\
	if (node->__FIELD.rb_right) {					\
		c = rb_entry(node->__FIELD.rb_right, __STRUCT, __FIELD);\
		if (c->__AUGMENTED > m)					\
			m = c->__AUGMENTED;				\
	}								\
	if (exit && node->__AUGMENTED == m)				\
		return true;						\
	node->__AUGMENTED = m;						\
	return false;							\
}									\
									\
RB_DECLARE_CALLBACKS(__STATIC, __NAME, __STRUCT, __FIELD,		\
		     __AUGMENTED, __NAME##_compute_max)


extern void lkpi_rb_insert_augmented(struct rb_node *node,
    struct rb_root *root, const struct rb_augment_callbacks *callbacks);
extern void lkpi_rb_erase_augmented(struct rb_node *node,
    struct rb_root *root, const struct rb_augment_callbacks *callbacks);

#define rb_insert_augmented(node, root, callbacks) \
    lkpi_rb_insert_augmented(node, root, callbacks)
#define rb_erase_augmented(node, root, callbacks) \
    lkpi_rb_erase_augmented(node, root, callbacks)

#endif /* _LINUXKPI_LINUX_RBTREE_AUGMENTED_H_ */
