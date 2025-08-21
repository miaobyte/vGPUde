#ifndef  TRIEKV_H
#define TRIEKV_H

#include <stddef.h>
#include <stdint.h>
#include "stdutil/bytes.h"


// 函数声明
void triekv_setmeta(const void* pool,const size_t pool_size, const size_t chartype,const size_t dataoffset_size, const size_t indexoffset_size);

void triekv_set(const bytes_t pool,const bytes_t key,const bytes_t value);
bytes_t triekv_get(const bytes_t pool,const bytes_t key);
void triekv_del(const bytes_t pool,const bytes_t key);

#endif // TRIEKV_H