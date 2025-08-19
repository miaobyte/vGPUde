#include <stdio.h>

#include "stdutil/triekv.h"

#define POOL_SIZE (1 << 20) // 1MB内存池

void test_meta(char* mem_pool){
     // 初始化内存池
    triekv_setmeta(mem_pool, POOL_SIZE,37,4,4);
}

void updatekey(const char *key, size_t keylen, uint8_t *mapped_key) {
    if (!key || !mapped_key) return;

    for (size_t i = 0; i < keylen; i++) {
        char c = key[i];

        // 映射规则：a-z -> 0-25, 0-9 -> 26-35, '_' -> 36
        if (c >= 'a' && c <= 'z') {
            mapped_key[i] = c - 'a'; // 映射到 0-25
        } else if (c >= '0' && c <= '9') {
            mapped_key[i] = c - '0' + 26; // 映射到 26-35
        } else if (c == '_') {
            mapped_key[i] = 36; // 映射到 36
        } else {
            mapped_key[i] = 0xFF; // 非法字符映射为 0xFF
        }
    }
}

void test_set(char* mem_pool) {
    char key[] = "example_key";
    uint8_t mapped_key[sizeof(key)];
    updatekey(key, sizeof(key) - 1, mapped_key); // 更新键的映射
    char value[] = "example_value";
    
    // 设置键值对
    triekv_set(mem_pool, mapped_key, sizeof(mapped_key), value, sizeof(value));
}

int main() {
    char mem_pool[POOL_SIZE];
    
    test_meta(mem_pool);
    test_set(mem_pool);
    return 0;
}