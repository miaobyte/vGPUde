#ifndef STDUTIL_BOXOFBOX_H
#define STDUTIL_BOXOFBOX_H

#include <stddef.h>
#include <stdint.h>

typedef struct
{
    uint64_t total_size;   // 总内存大小，不可变
    uint64_t block_size;   // 每个块的大小
    uint64_t max;
    uint64_t min;
    void *boxstart; // 指向块内存起始位置
} __attribute__((packed)) boxofbox_meta;

typedef struct
{
    int8_t boxorobj; //>0时，标识box_level
    uint64_t total_size;
    uint8_t used[4]; //32个bit,使用16个bit2,意味着有16个子节点（obj或box）
    // 0=不可用
    // 1=obj_start
    // 2=obj_continued
    // 3=box

    //l1,min=1,max=16-sizeof(obj)=5
    //l2 min=16-sizeof(obj),max=256-sizeof(boxofbox)
    //l3 min=256-sizeof(boxofbox),max=16^3-sizeof(boxofbox)
    uint64_t levelmax;//随着开始使用减少，需要同步到父节点的childmax
    uint64_t levelmin;//随着使用减少，需要同步到父节点的childmax
    uint64_t childmax;
    uint64_t childmin;
} __attribute__((packed)) boxofbox;

typedef struct
{
    int8_t boxorobj; //=-1 是obj
    uint64_t total_size;
} __attribute__((packed)) obj;

void boxofbox_init(boxofbox_meta* meta, size_t size, size_t block_size,void *boxstart);
void *boxofbox_alloc(boxofbox_meta* meta, size_t size);
void boxofbox_free(boxofbox_meta* meta, void *ptr);

#endif //STDUTIL_BOXOFBOX_H