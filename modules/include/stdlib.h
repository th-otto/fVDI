#include "libkern.h"

/* From fvdi/engine/utility.c */
extern void *DRIVER_EXPORT malloc(size_t size);
extern void *realloc(void *addr, size_t  new_size);
extern void DRIVER_EXPORT free(void *addr);

void qsort(void *base, long nmemb, long size, int (*compar) (const void *, const void *));

