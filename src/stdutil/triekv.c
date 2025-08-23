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
    //int64_t childs[char_type]; // 存储子节点的blockid,<0表示没有子节点
} TrieKV_indexnode;
static int TrieKV_indexnode_init(const TrieKVMeta* meta,const TrieKV_indexnode* node){
    if (!node) {
        LOG("TrieKV_indexnode_init node is NULL");
        return -1;
    }
    int64_t* p=(int64_t*)node;
    *p=-1;// 初始化hasobj_offset为-1，表示没有对象
    p++;
    for (size_t i = 0; i < meta->char_type; i++) {
        p[i] = -1; // 初始化所有子节点
    }
    return 0;
}
static size_t TrieKV_indexnode_size(const TrieKVMeta* meta)
{
    return sizeof(TrieKV_indexnode) + meta->char_type * sizeof(int64_t);
}
static int64_t* TrieKV_indexnode_childblockid(const TrieKVMeta* meta,const TrieKV_indexnode* node,int8_t i){
    if (i >= meta->char_type) {
        LOG("TrieKV_indexnode_childblockid,Index %u out of range", i);
        return NULL;
    }
    return (int64_t*)node+1+ i;
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
        LOG("pool size %lu is too small for TrieKVMeta %zu", pool_size, sizeof(TrieKVMeta));
        return -1;
    }
    TrieKVMeta *meta = (TrieKVMeta *)pool;
    meta->pool_size = pool_size;
    meta->char_type = chartype;
 
    LOG("meta size: %zu", sizeof(TrieKVMeta));

    size_t index_size = TrieKV_indexnode_size(meta);

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
    TrieKV_indexnode_init(meta, root_node); // 初始化根节点
    LOG("TrieKV root node initialized");
    return 0;
}

void triekv_set(const bytes_t pool, const bytes_t key, const uint64_t value)
{
    if (!pool.data || key.len <= 0)
        return;

    TrieKVMeta *meta = (TrieKVMeta *)pool.data;
    TrieKV_indexnode *root_node = block_data(&meta->blocks, 0);
    TrieKV_indexnode *cur_node = root_node;

    for (size_t i = 0; i < key.len; i++)
    {
        // 当前字符的索引
        uint8_t char_index = *(key.data+i);
        int64_t* childi =TrieKV_indexnode_childblockid(meta,cur_node,char_index);
        if (*childi<0){
            // 如果没有子节点，分配一个新的节点
            block_t *new_block = blocks_alloc(&meta->blocks);
            if (!new_block)
            {
                return;
            }
            cur_node = block_data(&meta->blocks, new_block->id);
            TrieKV_indexnode_init(meta, cur_node); // 初始化新节点
            *childi = new_block->id; // 更新当前节点的子节点指针
        }
        else
        {
            cur_node = block_data(&meta->blocks, *childi); // 跳转到子节点
        }
    }

    // 设置当前节点的hasobj_offset为value
    int64_t *hasobj_offset = (int64_t *)cur_node;
    if (*hasobj_offset < 0){
        LOG("set at %lu", value);
        *hasobj_offset = value; // 设置对象偏移
    }else{
        LOG("kv exists, updating value");
        *hasobj_offset = value; // 更新对象偏移
    }
}

static void TrieKV_traverse_dfs(const TrieKVMeta *meta, const TrieKV_indexnode *node, char *key_buffer, size_t depth, BYTES_FUNC(*func)) {
    if (!node) return;

    // 如果当前节点有值，调用回调函数处理键
    int64_t *hasobj_offset = (int64_t *)node;
    if (*hasobj_offset >= 0) {
        key_buffer[depth] = '\0'; // 终止字符串
        bytes_t key = { .data = (uint8_t *)key_buffer, .len = depth };
        LOG("Found key at depth %zu with offset %ld", depth, *hasobj_offset);
        //func(key); 
    }

    // 遍历所有子节点
    for (size_t i = 0; i < meta->char_type; i++) {
        int64_t *child_id = TrieKV_indexnode_childblockid(meta, node, i);
        if (child_id && *child_id >= 0) {
            TrieKV_indexnode *child_node = block_data(&meta->blocks, *child_id);
            key_buffer[depth] = (char)i; // 将当前字符加入键
            TrieKV_traverse_dfs(meta, child_node, key_buffer, depth + 1, func);
        }
    }
}

void triekv_keys(const bytes_t pool, const bytes_t prefix, BYTES_FUNC(*func)) {
    if (!pool.data || !func) {
        LOG("Invalid arguments to triekv_keys");
        return;
    }

    TrieKVMeta *meta = (TrieKVMeta *)pool.data;
    TrieKV_indexnode *root_node = block_data(&meta->blocks, 0);
    if (!root_node) {
        LOG("Root node is NULL");
        return;
    }

    char key_buffer[256]; // 假设最大键长度为 256
    size_t depth = 0;

    // 将前缀写入 key_buffer
    if (prefix.data && prefix.len > 0) {
        if (prefix.len >= sizeof(key_buffer)) {
            LOG("Prefix is too long");
            return;
        }
        memcpy(key_buffer, prefix.data, prefix.len);
        depth = prefix.len;

        // 遍历到前缀对应的节点
        TrieKV_indexnode *cur_node = root_node;
        for (size_t i = 0; i < prefix.len; i++) {
            uint8_t char_index = prefix.data[i];
            int64_t *child_id = TrieKV_indexnode_childblockid(meta, cur_node, char_index);
            if (!child_id || *child_id < 0) {
                LOG("Prefix not found");
                return; // 前缀不存在
            }
            cur_node = block_data(&meta->blocks, *child_id);
        }

        // 从前缀节点开始递归遍历
        TrieKV_traverse_dfs(meta, cur_node, key_buffer, depth, func);
    } else {
        // 如果没有前缀，从根节点开始遍历
        TrieKV_traverse_dfs(meta, root_node, key_buffer, depth, func);
    }
}

bytes_t triekv_get(const bytes_t pool, const bytes_t key){
    
}
void triekv_del(const bytes_t pool, const bytes_t key);