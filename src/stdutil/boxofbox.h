#ifndef STDUTIL_BOXOFBOX_H
#define STDUTIL_BOXOFBOX_H

#include <stddef.h>
#include <stdint.h>

typedef struct
{
    uint64_t control_size; // 总内存大小，不可变
    uint64_t min;
    uint64_t max;
    uint64_t levelmax[16];
    void *root; // 指向块内存起始位置
} __attribute__((packed)) boxofbox_meta;

typedef struct
{
    uint8_t boxtype;// 0=不可用,1=box可用但长度不完整,2=box长度完整,3=obj
    uint8_t boxlevel;//[0,16]
    union boxofbox
    {
       NODE_INT childs[16]; // 存储子节点的blockid,<0表示没有子节点
       uint64_t objsize; // obj的大小
    };  
} __attribute__((packed)) TrieKV_indexnode;

typedef struct
{
    int8_t boxorobj;
} __attribute__((packed)) box;// 1bytes

typedef struct
{
    box box;
    uint8_t used[4]; // 32个bit,使用16个bit2,意味着有16个子节点（obj或box）
    // 0=不可用
    // 1=box完整可用
    // 2=obj_start
    // 3=obj_continued

    uint8_t continued_max; //连续的最大空闲obj  level_[0,16],child_[0,16]
} __attribute__((packed)) boxofbox; // 1+4+1=5bytes

void boxofbox_init(boxofbox_meta *meta, size_t control_size, void *boxstart);
box* boxofbox_alloc(boxofbox_meta *meta, size_t size);
void boxofbox_free(boxofbox_meta *meta, void *ptr);

#endif // STDUTIL_BOXOFBOX_H