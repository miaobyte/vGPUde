#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "stdutil/triekv.h"
#include "stdutil/logutil.h"


typedef struct {
    uint64_t pool_size;    // 内存池大小
    uint16_t char_type;    // 字符类别数量
    uint8_t dataoffset_size; // 数据偏移量size,占用内存大小
    uint8_t indexoffset_size; // 索引偏移量size,占用内存大小
    
    //extra
    uint16_t index_size;
} TrieKVMeta;

void triekv_setmeta(const void* pool,const size_t pool_size, const size_t chartype,const size_t dataoffset_size, const size_t indexoffset_size){
    if( !pool ||pool_size<sizeof(TrieKVMeta)){
        return;
    }
    TrieKVMeta *meta = (TrieKVMeta *)pool;
    meta->pool_size=(uint64_t)pool_size;
    meta->char_type=(uint16_t)chartype;
    meta->dataoffset_size = (uint8_t)dataoffset_size;
    meta->indexoffset_size = (uint8_t)indexoffset_size;

    LOG("meta size: %zu",sizeof(TrieKVMeta));

    //extra
    meta->index_size= meta->dataoffset_size+meta->indexoffset_size*(2+chartype);

    LOG("index size: %u", meta->index_size);

}
/*
{   
    arrayhead
        uint64 element_size
        uint64 size
        uint64 freelist_next:
    arraydata[]
        freelist_pre
        freelist_next
        data
        
}
*/
void  list_init(void *list, const size_t element_size, const size_t size);
void  list_mark_use(void *list);
void  list_mark_ususe(void *list);
static void  list_append(void *list);
static void  list_dellast(void *list);

/*
{   
    node parent

    hasobj_offset
    node childs[chartype]
}
*/
void triekv_set(const bytes_t pool,const bytes_t key,const bytes_t value){
    if (!pool.data || !key.len<=0) return;

    TrieKVMeta *meta = (TrieKVMeta *)pool.data;
    void *trie_root = pool.data + sizeof(TrieKVMeta);
    void *cur_node = trie_root;

    for (size_t i = 0; i < key.len; i++) {
        //当前字符的索引
        uint8_t char_index = ((uint8_t *)key.data)[i];

        size_t node_offset = char_index * meta->indexoffset_size;
    }
}
bytes_t triekv_get(const bytes_t pool,const bytes_t key);
void triekv_del(const bytes_t pool,const bytes_t key);