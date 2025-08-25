#include "boxofbox.h"
#include "logutil.h"


// 向上取整到 alignment 的倍数
#define ROUND_UP(size, alignment) (((size) + (alignment) - 1) / (alignment) * (alignment))

// 向下取整到 alignment 的倍数
#define ROUND_DOWN(size, alignment) ((size) / (alignment) * (alignment))


void boxofbox_init(box_meta *meta, size_t control_size, void *boxstart)
{
    *meta = (box_meta){
        .control_size = control_size,
        .min = 1,
        .max = control_size - sizeof(box_t),
        .root = boxstart
    };
    for(int i=0;i<16;i++){
        meta->levelmax[i]=meta->max;
    }
    // 初始化根节点为空闲状态
    box_t *root = (box_t *)boxstart;
 

}

static box_t*  box_alloc(box_meta *meta,box_t* node, size_t size){
    if(!node) {
        LOG("Error: Node is NULL");
        return NULL;
    }

}
 
void boxofbox_free(box_meta *meta, void *ptr);