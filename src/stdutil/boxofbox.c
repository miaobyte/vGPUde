#include "boxofbox.h"
#include "logutil.h"


// 向上取整到 alignment 的倍数
#define ROUND_UP(size, alignment) (((size) + (alignment) - 1) / (alignment) * (alignment))

// 向下取整到 alignment 的倍数
#define ROUND_DOWN(size, alignment) ((size) / (alignment) * (alignment))


void boxofbox_init(boxofbox_meta *meta, size_t control_size, void *boxstart)
{
    *meta = (boxofbox_meta){
        .control_size = control_size,
        .min = 1,
        .max = control_size - sizeof(box),
        .root = boxstart
    };
    for(int i=0;i<16;i++){
        meta->levelmax[i]=meta->max;
    }
    // 初始化根节点为空闲状态
    box *root = (box *)boxstart;
    *root = (box){
        .boxorobj = 0,//0 biaoshi kongxian
    };

}

static box*  box_alloc(boxofbox_meta *meta,box* node, size_t size){
    if(!node) {
        LOG("Error: Node is NULL");
        return NULL;
    }
    if(node->boxorobj<0){
        LOG("Error: This node is an object, cannot allocate from it");
        return NULL;
    }
    if(node->boxorobj==0){
        
    }
}
box* boxofbox_alloc(boxofbox_meta *meta, size_t size){
    
}
void boxofbox_free(boxofbox_meta *meta, void *ptr);