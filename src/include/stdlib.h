#ifndef STDLIB_H
#define STDLIB_H

#include <stddef.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#define FIELD_SIZEOF(t, f) (sizeof(((t *)0)->f))

#define __ALIGN_MASK(x, mask)    ({ \
	typeof(mask) _mask = mask; \
	((x) + _mask) & ~_mask; \
})

#define ALIGN(x,a)              __ALIGN_MASK(x,(typeof(x))(a)-1UL)
#define ALIGN_UP(x,a)           ALIGN((x),(a))
#define ALIGN_DOWN(x,a)         ((x) & ~((typeof(x))(a)-1UL))
#define IS_ALIGNED(x,a)         (((x) & ((typeof(x))(a)-1UL)) == 0)

#define MIN(a, b) ({ \
	typeof(a) _a = a; \
	typeof(b) _b = b; \
	_a < _b ? _a : _b; \
})
#define MAX(a, b) ({ \
	typeof(a) _a = a; \
	typeof(b) _b = b; \
	_a > _b ? _a : _b; \
})

#if !defined(__PRE_RAM__)
void *memalign(size_t boundary, size_t size);
void *malloc(size_t size);
/* We never free memory */
static inline void free(void *ptr) {}
#endif

/* not sure romcc will know how to handle this. */
#ifndef __ROMCC__
static inline unsigned long div_round_up(unsigned int n, unsigned int d)
{
	return (n + d - 1) / d;
}
#endif

#endif /* STDLIB_H */
