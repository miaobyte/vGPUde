#ifndef STDUTIL_BOX_H
#define STDUTIL_BOX_H

#include <stddef.h>
#include <stdint.h>

#include "stdutil/block.h"

typedef struct
{
    uint64_t buddysize;//伙伴系统的总size
    void *rootbox;            // 指向块内存起始位置
    uint64_t box_size; // 总内存大小，不可变，内存长度必须=16^n*x,n=[1~]，x=[1,15]

    blocks_meta blocks;
} __attribute__((packed)) box_meta;

typedef enum
{
    BOX_UNUSED = 0,    // 未用（可以分配 obj、box）
    BOX_FORMATTED = 1, // box 已格式化
    OBJ_START = 2,     // obj_start
    OBJ_CONTINUED = 3  // obj_continued
} BoxState;
typedef struct
{
    uint64_t objsize;
} __attribute__((packed)) box_obj;

typedef struct 
{
    uint8_t state:2;// 0=未用（可以分配obj、box）,1=已格式化为box，2=obj
    int8_t continue_max:6; // 连续的最大空闲obj,[0~16]
} __attribute__((packed)) box_child;


typedef struct
{
    int32_t slotsid[16];   // 存储子节点的blockid,<0表示没有子节点
    box_child used_slots[16]; //直接存储子节点的状态
    int8_t childbox_obj_levelmin; // 子节点中最小的obj_level
    int8_t childbox_obj_levelmax; // 子节点中最大的obj_level

    uint8_t level_continued_max; // level连续的最大空闲obj,[0~15]
    uint8_t child_continued_max; // child连续的最大空闲obj,[0~15]
} __attribute__((packed)) box_used_body;
union box_body
{
    box_obj obj;
    box_used_body used_box_body;
};

typedef struct
{
    uint8_t state;         // 0=未用（可以分配obj、box）,1=已格式化为box，2=obj
    uint8_t level;         //[0,16]
    uint8_t avliable_slot; // 【2，16]
    int32_t parent;//parent_blockid
    union box_body body; // 32个bit,使用16个bit2,意味着有16个子节点（obj或box）

} __attribute__((packed)) box_head; //

int box_init(box_meta *meta,size_t buddysize, void *boxstart, size_t box_size);
void *box_alloc(box_meta *meta, size_t size);
void box_free(box_meta *meta, void *ptr);

#endif // STDUTIL_BOX_H