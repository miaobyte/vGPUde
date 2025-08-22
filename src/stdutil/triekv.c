#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "stdutil/triekv.h"
#include "stdutil/block.h"
#include "stdutil/logutil.h"

typedef struct
{
    uint64_t pool_size;       // 内存池大小
    uint16_t char_type;       // 字符类别数量

    // extra
    uint16_t index_size;

    // blocks_meta
    blocks_meta blocks;
} TrieKVMeta;

typedef struct
{
    int64_t hasobj_offset;   // 该节点是否有对象，<0表示没有，>0表示偏移
    //int64_t childs[char_type];     // 存储子节点的偏移量,<0表示没有子节点
} TrieKV_indexnode;
static int TrieKV_node_init(const TrieKVMeta* meta,const TrieKV_indexnode* node){
    if (!node) {
        LOG("TrieKV_node_init node is NULL");
        return -1;
    }
    int64_t* p=node;
    *p=-1;// 初始化hasobj_offset为-1，表示没有对象
    p++;
    for (size_t i = 0; i < meta->char_type; i++) {
        p[i] = -1; // 初始化所有子节点
    }
    return ;
}
static size_t TrieKV_node_size(const TrieKVMeta* meta)
{
    return sizeof(TrieKV_indexnode) + meta->char_type * sizeof(int64_t);
}
static int64_t* TrieKV_node_child(const TrieKVMeta* meta,const TrieKV_indexnode* node,int8_t i){
    if (i >= meta->char_type) {
        LOG("TrieKV_node_child Index %u out of range", i);
        return NULL;
    }
    return node +1+ i;
}

int triekv_setmeta(const void *pool, const uint64_t pool_size, const uint16_t chartype)
{
    if (!pool)
    {   
        LOG("pool is NULL");
        return -1;
    }
    if (pool_size < sizeof(TrieKVMeta))
    {
        LOG("pool size %zu is too small for TrieKVMeta %zu", pool_size, sizeof(TrieKVMeta));
        return -1;
    }
    TrieKVMeta *meta = (TrieKVMeta *)pool;
    meta->pool_size = pool_size;
    meta->char_type = chartype;
 
    LOG("meta size: %zu", sizeof(TrieKVMeta));

    size_t index_size = TrieKV_node_size(meta);

    LOG("index size: %u", meta->index_size);
    void* block_start = (void *)pool + sizeof(TrieKVMeta);
    init_blocks(block_start, pool_size - sizeof(TrieKVMeta), index_size, &meta->blocks);
    
    //rootnode
    blocks_alloc(&meta->blocks); // 分配根节点
    TrieKV_indexnode *root_node = block_data(&meta->blocks, 0);
    if (!root_node){
        LOG("Failed to allocate root node");
        return -1;
    }
    TrieKV_node_init(meta, root_node); // 初始化根节点
    LOG("TrieKV root node initialized");
}

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
 
    }
}
bytes_t triekv_get(const bytes_t pool, const bytes_t key);
void triekv_del(const bytes_t pool, const bytes_t key);