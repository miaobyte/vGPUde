#include "stdutil/box.h"
#include "stdutil/logutil.h"
#include "stdutil/bit2.h"
#include "stdutil/block.h"
#include "stdutil/aligned_size.h"

int box_init(box_meta *meta,size_t buddysize, void *rootbox, size_t box_size)
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
    
    *meta = (box_meta){
        .buddysize = buddysize,
        .rootbox = rootbox,
        .box_size = box_size,
    };
    void *block_start = (void *)meta +sizeof(box_meta);
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
        .level = rounded_size_t.power + 1,       // 根节点的数量级别（control_size/8后，再向上取log16
        .avliable_slot = rounded_size_t.multiple, // 初始有16个子节点可用
        .parent = -1,//root 没有parent
    };
    return 0;
}

static uint64_t box_find_alloc(box_meta *meta, box_head *node, AlignedSize objsize)
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
        node->state = BOX_FORMATTED;
        for (int i = 0; i < node->avliable_slot; i++)
        {
            node->body.used_box_body.slotsid[i] = -1; // 初始没有子节点,初始化后如果为-1，说明该区域内存不可用
            node->body.used_box_body.used_slots[i].state = BOX_UNUSED;
            node->body.used_box_body.used_slots[i].continue_max = 16;
        }
        node->body.used_box_body.level_continued_max= node->avliable_slot;
        node->body.used_box_body.childbox_obj_levelmin = -1; // 初始没有子节点
        node->body.used_box_body.childbox_obj_levelmax = -1;
        node->body.used_box_body.child_continued_max = 16;
    };

    if (node->state == BOX_FORMATTED)
    {
        if (objsize.power + 1 == node->level)
        {
            // 目标体量 属于当前level
           
            return 0; // 偏移量=0
        }
        else
        {

            box_find_alloc();
        }
    }
}
void *box_alloc(box_meta *meta, size_t size)
{
    AlignedSize aligned_size = align_to((size + 8 - 1) / 8, 16);
    aligned_size.aligned_value *= 8;

    box_head *root =block_data(&meta->blocks, 0);
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
    return box_find_alloc(meta, root, aligned_size);
}
void *box_alloc(box_meta *meta, size_t size);
void box_free(box_meta *meta, void *ptr);
