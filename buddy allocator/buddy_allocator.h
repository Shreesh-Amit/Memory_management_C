#ifndef BUDDY_ALLOCATOR
#define BUDDY_ALLOCATOR

#include <stdint.h>

void *umalloc(uint16_t bytes);
void ufree(void *ptr);
void print_freelist(void);
void heap_init(void);

#endif