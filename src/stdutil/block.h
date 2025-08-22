#ifndef __BLOCK_H__
#define __BLOCK_H__

#include <stddef.h>
#include <stdint.h>

#include "stdutil/logutil.h"
#include "stdutil/bytes.h"

typedef struct
{
    size_t total_size;   // 总内存大小，不可变
    size_t block_size;   // 每个块的大小
    size_t total_blocks; // 总块数
    size_t used_blocks;  // 已使用的块数
    size_t free_next_id; // 首个空闲块的id,if no free block, this is -1;
} __attribute__((packed)) blocks_t;

blocks_t *init_blocks(uint8_t *blocks_start, const size_t total_size, const size_t block_size);

typedef struct
{
    size_t id;
    uint8_t used;        // 0: free, 1: used
    size_t free_next_id; // -1,or id;
} __attribute__((packed)) block_t;

block_t *block_ptr(blocks_t *blocks, size_t id);

block_t blocks_alloc(blocks_t *blocks);
void blocks_free(blocks_t *blocks, size_t id);

#endif // __BLOCK_H__