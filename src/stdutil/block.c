#include "stdutil/block.h"

void layout_blocks(uint8_t *blocks_start, blocks_t blocks)
{
    uint8_t *p =blocks_start;
    // total_size
    *(size_t *)p = blocks.total_size;
    p += sizeof(blocks.total_size);

    // block_size
    *(size_t *)p = blocks.block_size;
    p += sizeof(blocks.block_size);

    // total_blocks
    *(size_t *)p = 0;
    p += sizeof(blocks.total_blocks);

    // used_blocks
    *(size_t *)p = 0;
    p += sizeof(blocks.used_blocks);

    // free_next_id
    *(size_t *)p = -1; // 初始化为 -1，表示没有空闲块，需要新增分配
}

void layout_block(uint8_t *block_start, block_t block)
{
    uint8_t *p = block_start;

    // block.id
    *(size_t *)p = block.id;
    p += sizeof(block.id);

    // block.used
    *p = block.used;
    p += sizeof(block.used);

    // block.free_next_id
    *(size_t *)p = block.free_next_id;
}

size_t block_ptr(blocks_t blocks, size_t id)
{
    size_t ptr = sizeof(blocks_t);
    if (id< 0){
            return ptr;
        }
    if (id >= blocks.total_blocks)
    {
        LOG("block id %zu out of range", id);
        return;
    }
    ptr += (sizeof(block) + blocks.block_size)(id - 1);
    return ptr;
}

static int blocks_append(blocks_t *blocks)
{
    // 计算新的总内存大小

    // 更新 blocks
}

block_t blocks_alloc(blocks_t *blocks);
{
    // 当 free_next_id < 0 时，表示没有空闲块，需要分配新的块
    // 当 free_next_id>=0 时，表示有空闲块，可以直接使用 free_next_id 指向的块

    block_t *block;

    // 如果没有空闲块
    if (blocks->free_next_id < 0)
    {
        LOG("No free blocks");
        size_t totalused_size = sizeof(blocks_t) + blocks->total_blocks * blocks->block_size;
        if totalused_size
            +blocks->block_size > blocks->total_size
            {
                LOG("out of memory");
                return -1;
            }
        else
        {
            LOG("append new block");

            block = (block_t *)((uint8_t *)blocks + totalused_size);
            *block = (block_t)
            {
                .id = blocks->total_blocks - 1,
                .used = 0,
                .free_next_id = -1;
            };

            blocks->total_blocks++;
            blocks->used_blocks++;
            return *block;
        }
    }

    // 获取空闲块的 ID
    size_t free_id = blocks->free_next_id;

    // 更新空闲块链表
    blocks->free_next_id = block_ptr(*blocks, free_id);

    // 更新已使用块数
    blocks->used_blocks++;

    // 初始化分配的块
    block.id = free_id;
    block.used = 1;
    block.free_next_id = (size_t)-1;

    layout_block(blocks->data + block_ptr(*blocks, free_id), block);

    return block;
}