把一个bytes数组的内存/文件/其他设备，转换为kv寻址

//meta区

index区和data区，如果共用一个内存区，那么为了避免冲突
方案1.需要index在左向右增长，data区在右侧，向左增长
方案2.index区和data区按比例切分

index区和data区，不共用一个内存区，各自独占一大段bytes空间


//index区
采用trie树，每个节点包含cnt个子节点。cnt索引可以对应字符。

typedef xint int64 // or int32/int16
#define Charcnt 
type struct indextrienode{
    xint hasobj_objptr// 指向data区的obj的偏移
    xint[Charcnt] nodes
    pre,next freelist *indextrienode
}
indextrienode是固定size


//data区
也采用trie树，但是为了实现套盒模型，每个盒子都可以装更小一级的盒子（盒子size=256^level）或对象(256^(level-1)<对象size<256^level)


type struct datatrienode{
    bitmap N/8
    childmin,levelmin,max //node需要维护自己管辖区的三个值，childmin可容纳的最小值，本node可容纳的最小值及最大值，便于malloc时搜索合适节点（这一点参考了slab分配器和伙伴系统）
    objlistnode data_freelist//显式空闲链表，malloc/free速度快
}
type struct objlistnode{
    pre,next objlistnode
    size_t objlen //objlen
    byte[objlen] data //真实对象
}




以下是标注接口定义

#ifndef  MEMPOOL_H
#define MEMPOOL_H

#include <stddef.h>
#include <stdint.h>

// 函数声明
void init_mem_pool( size_t size,size_t chartypecnt=37,size_t splitnum=256);
//size,总长度
//chartypecnt，字符类别数量
//splitnum，data区的分割基数

size_t value_malloc(size_t size);
//size,申请的长度

void value_free(size_t ptr);
//归还的地址，这里其实是偏移量

#endif // MEMPOOL_H