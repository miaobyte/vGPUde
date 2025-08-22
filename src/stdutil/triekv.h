#ifndef  TRIEKV_H
#define TRIEKV_H

#include <stddef.h>
#include <stdint.h>
#include "stdutil/bytes.h"


// 函数声明
void triekv_setmeta(const void *pool, const uint64_t pool_size, const uint16_t chartype, const uint8_t dataoffset_size, const uint8_t indexoffset_size);
void triekv_set(const bytes_t pool,const bytes_t key,const uint64_t valueptr);
bytes_t triekv_get(const bytes_t pool,const bytes_t key);
void triekv_del(const bytes_t pool,const bytes_t key);

#endif // TRIEKV_H