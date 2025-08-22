#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "stdutil/triekv.h"
#include "stdutil/logutil.h"

typedef struct
{
    uint64_t pool_size;       // 内存池大小
    uint16_t char_type;       // 字符类别数量
    uint8_t dataoffset_size;  // 数据偏移量size,占用内存大小
    uint8_t indexoffset_size; // 索引偏移量size,占用内存大小

    // extra
    uint16_t index_size;
} TrieKVMeta;

void triekv_setmeta(const void *pool, const uint64_t pool_size, const uint16_t chartype, const uint8_t dataoffset_size, const uint8_t indexoffset_size)
{
    if (!pool)
    {   
        LOG("pool is NULL");
        return;
    }
    if (pool_size < sizeof(TrieKVMeta))
    {
        LOG("pool size %zu is too small for TrieKVMeta %zu", pool_size, sizeof(TrieKVMeta));
        return;
    }
    TrieKVMeta *meta = (TrieKVMeta *)pool;
    meta->pool_size = pool_size;
    meta->char_type = chartype;
    meta->dataoffset_size =  dataoffset_size;
    meta->indexoffset_size =  indexoffset_size;

    LOG("meta size: %zu", sizeof(TrieKVMeta));

    // extra
    meta->index_size = meta->dataoffset_size + meta->indexoffset_size * (2 + chartype);

    LOG("index size: %u", meta->index_size);
}

/*
{
    node parent

    hasobj_offset
    node childs[chartype]
}
*/
void triekv_set(const bytes_t pool, const bytes_t key, const uint64_t value)
{
    if (!pool.data || !key.len <= 0)
        return;

    TrieKVMeta *meta = (TrieKVMeta *)pool.data;
    void *trie_root = pool.data + sizeof(TrieKVMeta);
    void *cur_node = trie_root;

    for (size_t i = 0; i < key.len; i++)
    {
        // 当前字符的索引
        uint8_t char_index = ((uint8_t *)key.data)[i];

        size_t node_offset = char_index * meta->indexoffset_size;
    }
}
bytes_t triekv_get(const bytes_t pool, const bytes_t key);
void triekv_del(const bytes_t pool, const bytes_t key);