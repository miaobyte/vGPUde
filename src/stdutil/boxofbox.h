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
    void *start; // 指向块内存起始位置
} __attribute__((packed)) boxofbox_meta;

typedef struct
{
    uint8_t boxorobj; //>=1时，标识box_level
    uint64_t total_size;
    uint8_t used[3]; //24个bit,只使用10个bit2
    // 0=box
    // 1=obj_start
    // 2=obj_continued

    //l2 min=10-sizeof(objinbox),max=100-sizeof(boxofbox)
    //l3 min=100-sizeof(boxofbox),max=100+100*100-sizeof(boxofbox)
    uint64_t levelmax;
    uint64_t levelmin;
    uint64_t childmin;
} __attribute__((packed)) boxofbox;

typedef struct
{
    uint8_t boxorobj; //=1 是obj
    uint64_t total_size;
} __attribute__((packed)) objinbox;

void boxofbox_init(void *mem, size_t size, size_t block_size);
void *boxofbox_alloc(void *mem, size_t size);
void boxofbox_free(void *mem, void *ptr);

#endif //STDUTIL_BOXOFBOX_H