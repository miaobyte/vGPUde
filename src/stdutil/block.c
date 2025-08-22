#include "stdutil/block.h"

void init_blocks(uint8_t *blocks_start,const size_t total_size,const size_t block_size)
{
    uint8_t *p =blocks_start;
    // total_size
    *(size_t *)p = total_size;
    p += sizeof(total_size);

    // block_size
    *(size_t *)p =block_size;
    p += sizeof(block_size);

    // total_blocks
    *(size_t *)p = 0;
    p += sizeof(total_size/block_size);

    // used_blocks
    *(size_t *)p = 0;
    p += sizeof(size_t);

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

block_t* block_ptr(blocks_t *blocks, size_t id)
{
    if (id >= (blocks->total_blocks)) {
        LOG("block id %zu out of range", id);
        return NULL; // Return NULL for invalid id
    }
    uint8_t* ptr = (uint8_t*)blocks + sizeof(blocks_t);
    ptr += (sizeof(block_t) + blocks->block_size) * id;
    return (block_t*)ptr;
}

block_t blocks_alloc(blocks_t *blocks) {
    if (blocks->free_next_id == (size_t)-1) {
        LOG("No free blocks available,need to append a new block.");
        size_t totalused_size = sizeof(blocks_t) + blocks->total_blocks * blocks->block_size;
        if (totalused_size + blocks->block_size > blocks->total_size) {
            LOG("Out of memory. %zu(totalused_size)= %zu(blocks_t)+ %zu(total_blocks)*%zu(block_size),when total_size %zu",
                totalused_size,  sizeof(blocks_t),blocks->total_blocks,blocks->block_size, blocks->total_size);
            block_t block;
            block.id = (size_t)-1; // Return invalid block
            return block;
        } else {
            
            block_t* new_block = (block_t*)((uint8_t*)blocks + totalused_size);
            *new_block = (block_t){
                .id = blocks->total_blocks,
                .used = 1,
                .free_next_id = (size_t)-1,
            };
            blocks->total_blocks++;
            blocks->used_blocks++;

            LOG("append new block %zu,blocks usage: %zu / %zu",
                 new_block->id,blocks->used_blocks,blocks->total_blocks);
            return *new_block;
        }
    }

    size_t free_id = blocks->free_next_id;
    block_t* free_block = block_ptr(blocks, free_id);
    free_block->used = 1;
    blocks->free_next_id = free_block->free_next_id;
    blocks->used_blocks++;

    LOG("Reusing free block. Block ID: %zu, Used blocks: %zu, Free blocks: %zu",
        free_id, blocks->used_blocks, blocks->total_blocks - blocks->used_blocks);

    return *free_block;
}

void blocks_free(blocks_t *blocks,size_t id){
    LOG("block id %zu ->free", id);
    if (id >= blocks->total_blocks) {
        LOG("block id %zu out of range", id);
        return;
    }
    block_t* free_block = block_ptr(blocks, id);
    free_block->used = 0;
    free_block->free_next_id= blocks->free_next_id;

    blocks->free_next_id=free_block->id;
    blocks->used_blocks--;
}