#ifndef __BLOCK_H__
#define __BLOCK_H__

#include <stddef.h>
#include <stdint.h>

#include "stdutil/logutil.h"
#include "stdutil/bytes.h"

typedef struct
{
    uint64_t total_size;   // 总内存大小，不可变
    uint64_t block_size;   // 每个块的大小
    uint64_t total_blocks; // 总块数
    uint64_t used_blocks;  // 已使用的块数
    int64_t free_next_id; // 首个空闲块的id,if no free block, this is -1;
    void *start; // 指向块内存起始位置
} __attribute__((packed)) blocks_meta_t;

int blocks_init(const uint64_t total_size, const uint64_t block_size,blocks_meta_t* blocks );

/*
下一步优化点

block_t 结构体中的 id 字段可以考虑去掉，因为它可以通过计算块的起始地址来推导出来，减少内存开销。    
*/
typedef struct
{
    uint64_t id;
    uint8_t used;        // 0: free, 1: used
    int64_t free_next_id; // -1,or id;
} __attribute__((packed)) block_t;

block_t* blocks_alloc(blocks_meta_t* meta);
void blocks_free(blocks_meta_t *meta,const uint64_t block_id);

block_t *block_ptr(const blocks_meta_t* meta,const uint64_t block_id);
void *block_data(const blocks_meta_t* meta,const uint64_t block_id);
block_t *block_bydata(const void* block_data);

#endif // __BLOCK_H__