#ifndef TRIEKV_H
#define TRIEKV_H

#include <stddef.h>
#include <stdint.h>

// 函数声明
void init_mem_pool(void *pool, size_t size);
void* value_malloc(size_t size);
void value_free(void *ptr);

#endif // TRIEKV_H