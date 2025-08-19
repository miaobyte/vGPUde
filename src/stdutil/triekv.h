#ifndef  TRIEKV_H
#define TRIEKV_H

#include <stddef.h>
#include <stdint.h>

// 函数声明
void triekv_setmeta(void *pool,const size_t size,const size_t chartype);
void triekv_getmeta(void *pool, size_t *size,size_t *chartype);
void* value_malloc(void *pool,size_t size);
void value_free(void *pool,void *ptr);

#endif // TRIEKV_H