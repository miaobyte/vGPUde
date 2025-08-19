#ifndef  TRIEKV_H
#define TRIEKV_H

#include <stddef.h>
#include <stdint.h>

// 函数声明
void triekv_setmeta(void *pool, const size_t size, const size_t chartype,const size_t dataoffset_size, const size_t indexoffset_size);

void triekv_set(void *pool,const void *key, const size_t keylen,const void *value, const size_t valuelen);
void triekv_get(const void *pool, const void *key, const size_t keylen, void *value, size_t *valuelen);
void triekv_del(void *pool, const void *key, const size_t keylen);

#endif // TRIEKV_H