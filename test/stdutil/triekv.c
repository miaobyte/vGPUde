#include "../../src/stdutil/triekv.h"

#define POOL_SIZE (1 << 20) // 1MB内存池

int main() {
    char mem_pool[POOL_SIZE];
    
    // 初始化内存池
    init_mem_pool(mem_pool, POOL_SIZE);
    
    // 分配内存
    int *arr = (int*)value_malloc(1024 * sizeof(int));
    char *str = (char*)value_malloc(128);
    
    // 使用内存
    // ...
    
    // 释放内存
    value_free(arr);
    value_free(str);
    
    return 0;
}