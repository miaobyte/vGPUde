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
} __attribute__((packed)) blocks_meta;

int init_blocks(void *block_start,const uint64_t total_size, const uint64_t block_size,blocks_meta* blocks );

typedef struct
{
    uint64_t id;
    uint8_t used;        // 0: free, 1: used
    int64_t free_next_id; // -1,or id;
} __attribute__((packed)) block_t;

block_t *block_ptr(const blocks_meta* blocks,const uint64_t id);
void *block_data(const blocks_meta* blocks,const uint64_t id);
block_t* blocks_alloc(blocks_meta* blocks);
void blocks_free(blocks_meta *blocks,const uint64_t id);

#endif // __BLOCK_H__