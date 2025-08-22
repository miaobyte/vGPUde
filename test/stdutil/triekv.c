#include <stdio.h>

#include "stdutil/triekv.h"

#define POOL_SIZE (1 << 20) // 1MB内存池

void test_meta(bytes_t pool) {
     // 初始化内存池
    triekv_setmeta(pool.data, pool.len,37);
}

void updatekey(const bytes_t key,const bytes_t mapped_key) {
    if (!key.data || !mapped_key.data) return;

    for (size_t i = 0; i < key.len; i++) {
        char c = key.data[i];

        // 映射规则：a-z -> 0-25, 0-9 -> 26-35, '_' -> 36
        if (c >= 'a' && c <= 'z') {
            mapped_key.data[i] = c - 'a'; // 映射到 0-25
        } else if (c >= '0' && c <= '9') {
            mapped_key.data[i] = c - '0' + 26; // 映射到 26-35
        } else if (c == '_') {
            mapped_key.data[i] = 36; // 映射到 36
        } else {
            mapped_key.data[i] = 0xFF; // 非法字符映射为 0xFF
        }
    }
}

void test_set(bytes_t mem_pool) {
    bytes_t key =BYTES_LITERAL("example_key");
    BYTES_BUFFER(mapped_key,key.len);

    updatekey(key, mapped_key); // 更新键的映射
    bytes_t value=BYTES_LITERAL("example_value");
    
    // 设置键值对
    triekv_set(mem_pool, mapped_key, value.data);
}

int main() {
    
    char mem_pool[POOL_SIZE];
    bytes_t pool = { .data = mem_pool, .len = POOL_SIZE };

    test_meta(pool);
    //test_set(pool);
    return 0;
}