#include "stdutil/block.h"

blocks_t *init_blocks(uint8_t *blocks_start, const size_t total_size, const size_t block_size)
{
    if (total_size < sizeof(blocks_t) + sizeof(block_t))
    {
        LOG("Total size %zu is too small for blocks_t and block_t structures", total_size);
        return NULL;
    }
    blocks_t *blocks = (blocks_t *)blocks_start;
    blocks->total_size = total_size;
    blocks->block_size = block_size;
    blocks->total_blocks = 0;
    blocks->used_blocks = 0;
    blocks->free_next_id = -1; // 初始化为 -1，表示没有空闲块，需要新增分配
    return blocks;
}

block_t *block_ptr(blocks_t *blocks, size_t id)
{
    if (id >= (blocks->total_blocks))
    {
        LOG("block id %zu out of range", id);
        return NULL; // Return NULL for invalid id
    }
    uint8_t *ptr = (uint8_t *)blocks + sizeof(blocks_t);
    ptr += (sizeof(block_t) + blocks->block_size) * id;
    return (block_t *)ptr;
}

block_t blocks_alloc(blocks_t *blocks)
{
    if (blocks->free_next_id == (size_t)-1)
    {
        size_t totalused_size = sizeof(blocks_t) + blocks->total_blocks * blocks->block_size;
        if (totalused_size + blocks->block_size > blocks->total_size)
        {
            LOG("Out of memory. %zu(totalused_size)= %zu(blocks_t)+ %zu(total_blocks)*%zu(block_size),when total_size %zu",
                totalused_size, sizeof(blocks_t), blocks->total_blocks, blocks->block_size, blocks->total_size);
            block_t block;
            block.id = -1; // Return invalid block
            return block;
        }
        else
        {
            blocks->total_blocks++;
            blocks->used_blocks++;

            block_t *new_block = block_ptr(blocks, blocks->total_blocks - 1);
            *new_block = (block_t){
                .id = blocks->total_blocks - 1,
                .used = 1,
                .free_next_id = (size_t)-1,
            };

            LOG("append new block %zu,blocks usage: %zu / %zu",
                new_block->id, blocks->used_blocks, blocks->total_blocks);
            return *new_block;
        }
    }

    size_t free_id = blocks->free_next_id;
    block_t *free_block = block_ptr(blocks, free_id);
    free_block->used = 1;
    blocks->free_next_id = free_block->free_next_id;
    free_block->free_next_id = (size_t)-1;
    blocks->used_blocks++;

    LOG("Reusing free block %zu, Used blocks: %zu/%zu",
        free_id, blocks->used_blocks, blocks->total_blocks);

    return *free_block;
}

void blocks_free(blocks_t *blocks, size_t id)
{
    LOG("block id %zu ->free", id);
    if (id >= blocks->total_blocks)
    {
        LOG("block id %zu out of range", id);
        return;
    }
    block_t *free_block = block_ptr(blocks, id);
    free_block->used = 0;
    // 空闲链表，追加到链表头
    free_block->free_next_id = blocks->free_next_id;
    blocks->free_next_id = free_block->id;

    blocks->used_blocks--;
}