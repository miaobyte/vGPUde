#include "triekv.h"

#define ALIGNMENT 8
#define MIN_BLOCK_SIZE 32

static inline uint64_t* BLK_HEADER(void* ptr) {
    return (uint64_t*)ptr;
}

static inline uint64_t* BLK_FOOTER(void* ptr) {
    return (uint64_t*)((char*)ptr + BLK_SIZE(ptr) - 8);
}

static inline void* BLK_NEXT(void* ptr) {
    return (void*)((char*)ptr + BLK_SIZE(ptr));
}

static inline void* BLK_PREV(void* ptr) {
    return (void*)((char*)ptr - BLK_PRED_SIZE(ptr));
}

static inline size_t BLK_SIZE(void* ptr) {
    return *BLK_HEADER(ptr) & ~0x7;
}

static inline int BLK_ALLOC(void* ptr) {
    return *BLK_HEADER(ptr) & 0x1;
}

static inline void SET_BLK(void* ptr, size_t size, int alloc) {
    *BLK_HEADER(ptr) = size | alloc;
    *BLK_FOOTER(ptr) = size | alloc;
}

static inline size_t PRED_SIZE(void* ptr) {
    return *(uint64_t*)((char*)ptr - 8) & ~0x7;
}

static inline size_t BLK_PRED_SIZE(void* ptr) {
    return *(uint64_t*)((char*)ptr - 8) & ~0x7;
}

// 全局变量
static void *heap_start = NULL;
static void *free_list_head = NULL;
static size_t heap_size = 0;

/* 初始化内存池 */
void init_mem_pool(void *pool, size_t size) {
    if (size < 64) return;  // 最小内存池要求

    heap_start = pool;
    heap_size = size;
    free_list_head = NULL;

    // 初始化序言块 (16B)
    uint64_t *prologue = (uint64_t*)heap_start;
    prologue[0] = 16 | 1;    // 头部
    prologue[1] = 16 | 1;    // 尾部

    // 初始化结语块 (8B)
    uint64_t *epilogue = (uint64_t*)((char*)heap_start + size - 8);
    *epilogue = 0 | 1;

    // 创建初始空闲块
    void *first_block = (char*)heap_start + 16;
    size_t block_size = size - 16 - 8;  // 总空间-序言-结语
    
    SET_BLK(first_block, block_size, 0);  // 初始化块
    
    // 将空闲块加入链表
    *(void**)((char*)first_block + 8) = NULL;  // next
    *(void**)((char*)first_block + 16) = NULL; // prev
    free_list_head = first_block;
}

/* 链表操作：从空闲链表删除块 */
static void remove_free_block(void *block) {
    void *prev = *(void**)((char*)block + 16);
    void *next = *(void**)((char*)block + 8);

    if (prev) *(void**)((char*)prev + 8) = next;
    else free_list_head = next;
    
    if (next) *(void**)((char*)next + 16) = prev;
}

/* 链表操作：将空闲块插入表头 */
static void insert_free_block(void *block) {
    *(void**)((char*)block + 8) = free_list_head;
    *(void**)((char*)block + 16) = NULL;
    if (free_list_head) {
        *(void**)((char*)free_list_head + 16) = block;
    }
    free_list_head = block;
}

/* 合并空闲块 */
static void *coalesce(void *block) {
    uint64_t pred_alloc = (block > (void*)((char*)heap_start + 16)) ? 
                          (PRED_SIZE(block) & 0x1) : 1;
    uint64_t next_alloc = BLK_ALLOC(BLK_NEXT(block));
    size_t size = BLK_SIZE(block);

    // 合并前块和/或后块
    if (!pred_alloc) {
        remove_free_block(BLK_PREV(block));
        size += PRED_SIZE(block);
        block = BLK_PREV(block);
    }
    
    if (!next_alloc) {
        remove_free_block(BLK_NEXT(block));
        size += BLK_SIZE(BLK_NEXT(block));
    }
    
    SET_BLK(block, size, 0);  // 创建合并后的大块
    return block;
}

/* 内存分配 */
void *value_malloc(size_t size) {
    if (!heap_start || size == 0) return NULL;
    
    // 对齐并计算实际所需空间
    size_t asize = (size + 7) & ~7;  // 8字节对齐
    size_t total_size = asize + 16;   // 头尾空间
    if (total_size < MIN_BLOCK_SIZE) total_size = MIN_BLOCK_SIZE;
    
    // 空闲链表搜索 (首次适应)
    void *cur = free_list_head;
    void *best_fit = NULL;
    
    while (cur) {
        size_t cur_size = BLK_SIZE(cur);
        // 精确匹配或可分割
        if (cur_size >= total_size) {
            best_fit = cur;
            if (cur_size - total_size < MIN_BLOCK_SIZE) break;
        }
        cur = *(void**)((char*)cur + 8);
    }
    
    if (!best_fit) return NULL;  // 空间不足
    
    size_t best_size = BLK_SIZE(best_fit);
    
    // 块分割 (剩余空间需足够)
    if (best_size - total_size >= MIN_BLOCK_SIZE) {
        // 创建新空闲块
        void *new_block = (char*)best_fit + total_size;
        SET_BLK(new_block, best_size - total_size, 0);
        
        // 将新块加入空闲链表
        *(void**)((char*)new_block + 8) = *(void**)((char*)best_fit + 8);
        *(void**)((char*)new_block + 16) = *(void**)((char*)best_fit + 16);
        if (*(void**)((char*)best_fit + 16)) {
            *(void**)((char*)*(void**)((char*)best_fit + 16) + 8) = new_block;
        }
        if (*(void**)((char*)best_fit + 8)) {
            *(void**)((char*)*(void**)((char*)best_fit + 8) + 16) = new_block;
        }
        if (free_list_head == best_fit) free_list_head = new_block;
        
        best_size = total_size;
    } else {
        // 整块分配，从空闲链表删除
        remove_free_block(best_fit);
    }
    
    // 设置分配块
    SET_BLK(best_fit, best_size, 1);
    return (char*)best_fit + 8;  // 返回有效载荷地址
}

/* 内存释放 */
void value_free(void *ptr) {
    if (!ptr || ptr < (void*)heap_start || ptr >= (void*)((char*)heap_start + heap_size)) 
        return;
    
    void *block = (char*)ptr - 8;  // 定位块头
    if (!BLK_ALLOC(block)) return; // 避免重复释放
    
    // 设置空闲并合并
    SET_BLK(block, BLK_SIZE(block), 0);
    block = coalesce(block);
    
    // 插入空闲链表头部
    *(void**)((char*)block + 8) = free_list_head;
    *(void**)((char*)block + 16) = NULL;
    if (free_list_head) *(void**)((char*)free_list_head + 16) = block;
    free_list_head = block;
}