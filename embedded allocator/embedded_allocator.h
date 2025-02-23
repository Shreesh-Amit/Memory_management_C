#ifndef EMBEDDED_ALLOCATOR
#define EMBEDDER_ALLOCATOR

#include <stddef.h>

void heap_init(size_t size);
void *salloc(size_t size);
void sfree(void *ptr);
void print_freelist(void);

#endif 