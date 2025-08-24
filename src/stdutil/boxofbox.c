#include "boxofbox.h"

void boxofbox_init(boxofbox_meta *meta, size_t control_size, void *boxstart)
{
    *meta = (boxofbox_meta){
        .control_size = control_size,
        .min = 1,
        .max = control_size - sizeof(box),
        .root = boxstart
    };

    // 初始化根节点为空闲状态
    box *root = (box *)boxstart;
    *root = (box){
        .boxorobj = 0,//0 biaoshi kongxian
        .control_size = control_size - sizeof(box),
        .parent = NULL
    };
}

void *boxofbox_alloc(boxofbox_meta *meta, size_t size){
    
}
void boxofbox_free(boxofbox_meta *meta, void *ptr);