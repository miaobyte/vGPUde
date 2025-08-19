#include <stddef.h>
#include <stdint.h>

typedef struct {
    size_t pool_size;    // 内存池大小
    size_t char_type;    // 字符类别数量
} TrieKVMeta;

void triekv_setmeta(void *pool, const size_t size, const size_t chartype) {
    if (!pool) return;

    TrieKVMeta *meta = (TrieKVMeta *)pool;
    meta->pool_size = size;
    meta->char_type = chartype;
}

void triekv_getmeta(void *pool, size_t *size, size_t *chartype) {
    if (!pool   || !size || !chartype) return;
    TrieKVMeta *meta = (TrieKVMeta *)pool;
    *size = meta->pool_size;
    *chartype = meta->char_type;
}