#include "stdutil/block.h"

int blocks_init(const uint64_t total_size, const uint64_t block_size,blocks_meta_t* blocks )
{
    if (total_size < sizeof(block_t))
    {
        LOG("Total size %zu is too small for blocks_meta_t and block_t structures", total_size);
        return -1;
    }
    *blocks=(blocks_meta_t){
        .total_size = total_size,
        .block_size = block_size,
        .total_blocks = 0,
        .used_blocks = 0,
        .free_next_id = -1, // 初始化为 -1，表示没有空闲块，需要新增分配
    };
    return 0;
}

block_t *block_ptr(const blocks_meta_t* blocks,const uint64_t id)
{
    if (id >= (blocks->total_blocks))
    {
        LOG("block id %zu out of range", id);
        return NULL; // Return NULL for invalid id
    }
    void *ptr =blocks->start +(sizeof(block_t) + blocks->block_size) * id;
    return (block_t *)ptr;
}
void *block_data(const blocks_meta_t* blocks,const uint64_t id)
{
    void *ptr =block_ptr(blocks,id);
    if(!ptr) return NULL;
    ptr += sizeof(block_t);
    return ptr;
}
block_t *block_bydata(const void* blockdata){
    void *ptr=(void*)blockdata - sizeof(block_t);
    return (block_t *)ptr;
}
block_t* blocks_alloc(blocks_meta_t* blocks)
{
    if (blocks->free_next_id == -1)
    {
        uint64_t totalused_size = blocks->total_blocks * blocks->block_size;
        if (totalused_size + blocks->block_size > blocks->total_size)
        {
            LOG("Out of memory. %zu(totalused_size)= %zu(blocks_meta_t)+ %zu(total_blocks)*%zu(block_size),when total_size %zu",
                totalused_size, sizeof(blocks_meta_t), blocks->total_blocks, blocks->block_size, blocks->total_size);
            return NULL;
        }
        else
        {
            blocks->total_blocks++;
            blocks->used_blocks++;

            block_t *block = block_ptr(blocks, blocks->total_blocks - 1);
            *block = (block_t){
                .id = blocks->total_blocks - 1,
                .used = 1,
                .free_next_id = -1,
            };

            LOG("append new block %zu,blocks usage: %zu / %zu",block->id, blocks->used_blocks, blocks->total_blocks);
            return block;
        }
    }
    uint64_t free_id = blocks->free_next_id;
    block_t *free_block = block_ptr(blocks,  blocks->free_next_id);
    blocks->free_next_id = free_block->free_next_id;
    blocks->used_blocks++;

    free_block->used = 1;
    free_block->free_next_id =-1;

    LOG("Reusing free block %zu, Used blocks: %zu/%zu",
        free_id, blocks->used_blocks, blocks->total_blocks);

    return free_block;
}

void blocks_free(blocks_meta_t *blocks,const uint64_t id)
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