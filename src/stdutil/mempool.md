定义trietree的每个node的子node为N=8

type struct trienode{

    //meta

    uint8 bitmap//bitmap，标记子node[i]是否真实存在
    uint8 objlist_len //一组对象放在一个node上，而对象的size必须大于这个trienode的管理区域的1/N，否则应该递归的放置在这个node的子node区域
    uint32*objlist_len objlist //动态数组，objlist_len最大不超过N
    uint32 dataareasize//区域的大小

    //data区，包含子node或obj数据
    ...
    ...
    obj区，或子node区
}

type struct pool{
    const minnodeobj=64*8
    uint64 poolsize=8
    trienode* root 
    uint8 levelmax
    uint32*levelmax levelfirsttrienode_list//每个level的首个空闲指针，用于下次分配对应level的地址。
}

以下是标注接口定义

#ifndef  MEMPOOL_H
#define MEMPOOL_H

#include <stddef.h>
#include <stdint.h>

// 函数声明
void init_mem_pool(void *pool, size_t size);
void* value_malloc(void *pool,size_t size);
void value_free(void *pool,void *ptr);

#endif // MEMPOOL_H

我们分析一下

init_mem_pool时，需要在pool建立一块sizeof(pool)的区域存储pool信息，并初始化rootnode

malloc时，需要根据size大小，计算level
根据pool的levelfirsttrienode指针，找到应该放置的node，
然后再在node的动态数组+1，修改node的bitmap，写入对象
然后，维护这个level的firsttrienode指针

free时，根据