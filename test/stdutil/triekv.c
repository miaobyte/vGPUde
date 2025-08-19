#include "stdutil/triekv.h"

#define POOL_SIZE (1 << 20) // 1MB内存池

void test_meta(char* mem_pool){
     // 初始化内存池
    triekv_setmeta(mem_pool, POOL_SIZE,37);
    
    size_t size, chartype;
    triekv_getmeta(mem_pool, &size, &chartype);
    
    // 输出内存池元数据
    printf("Memory Pool Size: %zu bytes\n", size);
    printf("Character Type Count: %zu\n", chartype);
}

int main() {
    char mem_pool[POOL_SIZE];
    
    test_meta(mem_pool);
    
    return 0;
}