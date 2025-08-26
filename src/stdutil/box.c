#include "stdutil/box.h"
#include "stdutil/block.h"
#include "stdutil/logutil.h"
#include "stdutil/block.h"
#include "stdutil/aligned_size.h"

typedef struct
{
    uint64_t buddysize; // 伙伴系统的总size
    void *rootbox;      // 指向块内存起始位置
    uint64_t box_size;  // 总内存大小，不可变，内存长度必须=16^n*x,n>=1，x=[1,15]

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
    uint8_t state : 2;       // 0=未用（可以分配obj、box）,1=已格式化为box，2=obj
    int8_t continue_max : 6; // 连续的最大空闲obj,[0~16]
} __attribute__((packed)) box_child;

typedef struct
{
    int32_t childs_blockid[16];   // 存储子节点的blockid,<0表示没有子节点
    box_child used_slots[16];     // 直接存储子节点的状态
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
    int32_t parent;        // parent_blockid
    union box_body body;   // 32个bit,使用16个bit2,意味着有16个子节点（obj或box）

} __attribute__((packed)) box_head; //

int box_init(void *metaptr, size_t buddysize, void *boxstart, size_t box_size)
{
    if (box_size % 8 != 0)
    {
        LOG("box_size must be aligned to 8. Given size: %zu", box_size);
        return -1;
    }
    AlignedSize rounded_size_t = align_to(box_size / 8, 16);
    if (box_size / 8 != rounded_size_t.aligned_value)
    {
        LOG("box_size must be aligned to 16. Given size: %zu, aligned size: %zu", box_size, rounded_size_t.aligned_value);
        return -1;
    }
    box_meta *meta = metaptr;
    *meta = (box_meta){
        .buddysize = buddysize,
        .rootbox = boxstart,
        .box_size = box_size,
    };
    void *block_start = metaptr + sizeof(box_meta);
    init_blocks(block_start, buddysize - sizeof(box_meta), sizeof(box_head), &meta->blocks);
    blocks_alloc(&meta->blocks); // 分配根节点
    box_head *root_boxhead = block_data(&meta->blocks, 0);
    if (!root_boxhead)
    {
        LOG("Failed to allocate root node");
        return -1;
    }
    *root_boxhead = (box_head){
        .state = BOX_UNUSED,
        .level = rounded_size_t.power + 1,        // 根节点的数量级别（control_size/8后，再向上取log16
        .avliable_slot = rounded_size_t.multiple, // 初始有16个子节点可用
        .parent = -1,                             // root 没有parent
    };
    return 0;
}

static void box_format(box_head *node, box_head *parent)
{
    node->state = BOX_FORMATTED;
    node->level = parent->level - 1;
    node->avliable_slot = 16;
    node->parent = block_by_data(parent)->id;
    for (int i = 0; i < 16; i++)
    {
        node->body.used_box_body.childs_blockid[i] = -1;
        node->body.used_box_body.used_slots[i].state = BOX_UNUSED;
        node->body.used_box_body.used_slots[i].continue_max = 16;
    }
    node->body.used_box_body.level_continued_max = node->avliable_slot;
    node->body.used_box_body.childbox_obj_levelmin = -1; // 初始没有子节点
    node->body.used_box_body.childbox_obj_levelmax = -1; // 初始没有子节点
    node->body.used_box_body.child_continued_max = 16;   // 一般只有顶层node才可能会不完整
}

static uint64_t box_find_alloc(box_meta *meta, box_head *node, box_head *parent, AlignedSize objsize)
{
    if (node->state == BOX_UNUSED)
    {
        if (node->level == objsize.power + 1 && objsize.multiple == node->avliable_slot)
        {
            // 可以直接分配
            node->state = OBJ_START;
            node->body.obj.objsize = objsize.aligned_value;

            //

            return 0; // 偏移量=0
        }
        // 分裂节点
        box_format(node, parent);
    }

    if (node->state == BOX_FORMATTED)
    {
        if (objsize.power + 1 == node->level)
        {
            // 目标体量 属于当前level
            return 0; // 偏移量=0
        }
        else if (objsize.power + 1 < node->level)
        {
            // 目标体量<当前level,查找子节点
            for (int i = 0; i < node->avliable_slot; i++)
            {
                if (node->body.used_box_body.used_slots[i].state == BOX_UNUSED)
                {
                    // 该子节点未被使用，可以分配
                    if (node->body.used_box_body.childs_blockid[i] < 0)
                    {
                        block_t *block = blocks_alloc(&(meta->blocks));
                        node->body.used_box_body.childs_blockid[i] = block->id;
                    }
                    box_head *childnode = block_data(&(meta->blocks), node->body.used_box_body.childs_blockid[i]);
                    return box_find_alloc(meta, childnode, node, objsize);
                }
                if (node->body.used_box_body.used_slots[i].state == BOX_FORMATTED)
                {
                    // 该子节点已经被格式化，判断size范围是否合适
                    if (objsize.power + 2 == node->level)
                    {
                        if (objsize.multiple <= node->body.used_box_body.used_slots[i].continue_max)
                        {
                            box_head *childnode = block_data(&(meta->blocks), node->body.used_box_body.childs_blockid[i]);
                            return box_find_alloc(meta, childnode, node, objsize);
                        }
                        else
                        {
                            continue;
                        }
                    }
                    else
                    {
                        box_head *childnode = block_data(&(meta->blocks), node->body.used_box_body.childs_blockid[i]);
                        return box_find_alloc(meta, childnode, node, objsize);
                    }
                }
            }
        }
    }
}
void *box_alloc(void *metaptr, size_t size)
{
    AlignedSize aligned_size = align_to((size + 8 - 1) / 8, 16);
    aligned_size.aligned_value *= 8;

    box_meta *meta = metaptr;
    box_head *root = block_data(&meta->blocks, 0);
    if (!root)
    {
        LOG("Error: Root is NULL");
        return NULL;
    };
    if (root->level < aligned_size.power + 1)
    {
        LOG("Error: Requested size %zu is too large for the box system", size);
        return NULL;
    }
    if (root->level == aligned_size.power + 1 && root->avliable_slot < aligned_size.multiple)
    {
        LOG("Error: Requested size %zu is too large for the box system", size);
        return NULL;
    }
    return meta->rootbox + box_find_alloc(meta, root, NULL, aligned_size);
}
void box_free(void *meta, void *ptr);
