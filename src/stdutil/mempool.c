#include <stddef.h>
#include <stdint.h>

// 内存块头 (8 字节)
typedef struct BlockHeader {
    uint16_t size;       // 当前块大小 (包括头)
    uint8_t is_free;     // 空闲标志
    struct BlockHeader* next;  // 空闲链表指针
} BlockHeader;

// 内存池控制结构 (32 字节)
typedef struct PoolControl {
    size_t total_size;      // 池总大小
    BlockHeader* free_list; // 空闲链表头
    uint8_t mem_pool[1];    // 实际内存起始点 (灵活数组)
} PoolControl;

// 初始化内存池
void init_mem_pool(void *pool, size_t size) {
    PoolControl *ctrl = (PoolControl*)pool;
    ctrl->total_size = size - sizeof(PoolControl);
    ctrl->free_list = (BlockHeader*)ctrl->mem_pool;
    
    // 初始化为单个空闲块
    ctrl->free_list->size = ctrl->total_size;
    ctrl->free_list->is_free = 1;
    ctrl->free_list->next = NULL;
}

// 内存分配
void* value_malloc(void *pool,size_t size) {
    PoolControl *ctrl = (PoolControl*)pool;
    size_t required = size + sizeof(BlockHeader);
    
    // 遍历空闲链表
    BlockHeader **cur = &ctrl->free_list;
    while (*cur) {
        if ((*cur)->is_free && (*cur)->size >= required) {
            // 找到合适块：尝试拆分
            if ((*cur)->size >= required + sizeof(BlockHeader) + 8) {
                BlockHeader *new_block = (BlockHeader*)((uint8_t*)(*cur) + required);
                new_block->size = (*cur)->size - required;
                new_block->is_free = 1;
                new_block->next = (*cur)->next;
                
                (*cur)->size = required;
                (*cur)->next = new_block;
            }
            
            (*cur)->is_free = 0;
            return (uint8_t*)(*cur) + sizeof(BlockHeader);
        }
        cur = &((*cur)->next);
    }
    return NULL; // 无合适块
}

// 内存释放 (自动整理碎片)
void value_free(void *pool,void *ptr) {
    if (!ptr) return;
    BlockHeader *block = (BlockHeader*)((uint8_t*)ptr - sizeof(BlockHeader));
    block->is_free = 1;
    
    PoolControl *ctrl = (PoolControl*)pool;
    
    // 向前合并碎片
    BlockHeader **cur = &ctrl->free_list;
    while (*cur) {
        BlockHeader *next_block = (BlockHeader*)((uint8_t*)*cur + (*cur)->size);
        
        // 检查当前块是否与下一个块相邻且空闲
        if (next_block == block && (*cur)->is_free) {
            (*cur)->size += block->size;
            (*cur)->next = block->next;
            block = *cur;  // 更新为合并后的块
        }
        
        // 检查当前块是否与释放块相邻
        BlockHeader *merged_block = (BlockHeader*)((uint8_t*)block + block->size);
        if (merged_block == *cur && (*cur)->is_free) {
            block->size += (*cur)->size;
            block->next = (*cur)->next;
            *cur = block;
        }
        
        cur = &((*cur)->next);
    }
}