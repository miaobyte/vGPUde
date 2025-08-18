#ifndef  MEMPOOL_H
#define MEMPOOL_H

#include <stddef.h>
#include <stdint.h>

// 函数声明
void init_mem_pool(void *pool, size_t size);
void* value_malloc(void *pool,size_t size);
void value_free(void *pool,void *ptr);

#endif // MEMPOOL_H