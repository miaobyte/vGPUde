#ifndef STDUTIL_BOXOFBOX_H
#define STDUTIL_BOXOFBOX_H

#include <stddef.h>
#include <stdint.h>

typedef struct
{
    int8_t boxorobj; //<0时为obj，=0时xianzhi，>0时weibox，标识box_level
    uint64_t control_size;
    void *parent; // 指向父级box
} __attribute__((packed)) box;


typedef struct
{
    uint64_t control_size; // 总内存大小，不可变
    uint64_t min;
    uint64_t max;
    box *root; // 指向块内存起始位置
} __attribute__((packed)) boxofbox_meta;


typedef struct
{
    box box;
    uint8_t used[4]; // 32个bit,使用16个bit2,意味着有16个子节点（obj或box）
    // 0=不可用
    // 1=box
    // 2=obj_start
    // 3=obj_continued

    // l1,min=1,max=16-sizeof(obj)=5
    // l2 min=16-sizeof(obj),max=256-sizeof(boxofbox)
    // l3 min=256-sizeof(boxofbox),max=16^3-sizeof(boxofbox)
    uint64_t levelmax; // 随着开始使用减少，需要同步到父节点的childmax
    uint64_t levelmin; // 随着使用减少，需要同步到父节点的childmax
    uint64_t childmax;
    uint64_t childmin;
} __attribute__((packed)) boxofbox;

void boxofbox_init(boxofbox_meta *meta, size_t control_size, void *boxstart);
void *boxofbox_alloc(boxofbox_meta *meta, size_t size);
void boxofbox_free(boxofbox_meta *meta, void *ptr);

#endif // STDUTIL_BOXOFBOX_H